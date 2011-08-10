/*
 * @video_stage.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * ihm vision thread implementation
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <VP_Api/vp_api.h>
#include <VP_Api/vp_api_error.h>
#include <VP_Api/vp_api_stage.h>
#include <VP_Api/vp_api_picture.h>
#include <VP_Stages/vp_stages_io_file.h>
#include <VP_Stages/vp_stages_i_camif.h>

#include <config.h>
#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_delay.h>
#include <VP_Stages/vp_stages_yuv2rgb.h>
#include <VP_Stages/vp_stages_buffer_to_picture.h>
#include <VLIB/Stages/vlib_stage_decode.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include <ardrone_tool/ardrone_tool.h>
#include <ardrone_tool/Com/config_com.h>
#ifdef __cplusplus
}
#endif

#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <sys/time.h> 
#include "UI/keyboard.h"

#ifndef RECORD_VIDEO
#define RECORD_VIDEO
#endif
#ifdef RECORD_VIDEO
#    include <ardrone_tool/Video/video_stage_recorder.h>
#endif

#include <ardrone_tool/Video/video_com_stage.h>
#include <ardrone_api.h>

#include "Video/video_stage.h"
IplImage* frontImgStream;
IplImage* bottomImgStream;

#define extractBottomImage 1 // Set to 1 to extract bottom image from image; 0 for single front image.

#define bottomWidth 176 //dimensions of bottom image
#define bottomHeight 144
#define USEWEBCAM 0 //set to 0 for drone's camera, 1 to use computer's webcam
#define WEBCAMINDEX 0 //webcam ID on computer

#define NB_STAGES 10

PIPELINE_HANDLE pipeline_handle;

IplImage *rgbHeader = NULL;  //[for colour]

int CAPTUREIMAGESTREAM = 0;

static uint8_t*  pixbuf_data       = NULL;
static vp_os_mutex_t  video_update_lock = PTHREAD_MUTEX_INITIALIZER;

C_RESULT output_gtk_stage_open( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  rgbHeader = cvCreateImageHeader(cvSize(QVGA_WIDTH, QVGA_HEIGHT), IPL_DEPTH_8U, 3);  //[for colour]
  rgbHeader->imageData = NULL;	     //[for colour]

  return (SUCCESS);
}

C_RESULT output_gtk_stage_transform( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  vp_os_mutex_lock(&video_update_lock);
 
  /* Get a reference to the last decoded picture */
  pixbuf_data      = (uint8_t*)in->buffers[0];
  rgbHeader->imageData = (char*)pixbuf_data;	//[for colour]
  vp_os_mutex_unlock(&video_update_lock);

  return (SUCCESS);
}

C_RESULT output_gtk_stage_close( void *cfg, vp_api_io_data_t *in, vp_api_io_data_t *out)
{
  return (SUCCESS);
}


const vp_api_stage_funcs_t vp_stages_output_gtk_funcs =
{
  NULL,
  (vp_api_stage_open_t)output_gtk_stage_open,
  (vp_api_stage_transform_t)output_gtk_stage_transform,
  (vp_api_stage_close_t)output_gtk_stage_close
};

DEFINE_THREAD_ROUTINE(video_stage, data)
{
  C_RESULT res;

  vp_api_io_pipeline_t    pipeline;
  vp_api_io_data_t        out;
  vp_api_io_stage_t       stages[NB_STAGES];

  vp_api_picture_t picture;

  video_com_config_t              icc;
  vlib_stage_decoding_config_t    vec;
  vp_stages_yuv2rgb_config_t      yuv2rgbconf;
#ifdef RECORD_VIDEO
  video_stage_recorder_config_t   vrc;
#endif
  /// Picture configuration
  picture.format        = PIX_FMT_YUV420P;

  picture.width         = QVGA_WIDTH;
  picture.height        = QVGA_HEIGHT;
  picture.framerate     = 30;

  picture.y_buf   = vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT     );
  picture.cr_buf  = vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT / 4 );
  picture.cb_buf  = vp_os_malloc( QVGA_WIDTH * QVGA_HEIGHT / 4 );

  picture.y_line_size   = QVGA_WIDTH;
  picture.cb_line_size  = QVGA_WIDTH / 2;
  picture.cr_line_size  = QVGA_WIDTH / 2;

  vp_os_memset(&icc,          0, sizeof( icc ));
  vp_os_memset(&vec,          0, sizeof( vec ));
  vp_os_memset(&yuv2rgbconf,  0, sizeof( yuv2rgbconf ));

  icc.com                 = COM_VIDEO();
  icc.buffer_size         = 100000;
  icc.protocol            = VP_COM_UDP;
  COM_CONFIG_SOCKET_VIDEO(&icc.socket, VP_COM_CLIENT, VIDEO_PORT, wifi_ardrone_ip);

  vec.width               = QVGA_WIDTH;
  vec.height              = QVGA_HEIGHT;
  vec.picture             = &picture;
  vec.block_mode_enable   = TRUE;
  vec.luma_only           = FALSE;

  yuv2rgbconf.rgb_format = VP_STAGES_RGB_FORMAT_RGB24;
#ifdef RECORD_VIDEO
  vrc.fp = NULL;
#endif

  pipeline.nb_stages = 0;

  stages[pipeline.nb_stages].type    = VP_API_INPUT_SOCKET;
  stages[pipeline.nb_stages].cfg     = (void *)&icc;
  stages[pipeline.nb_stages].funcs   = video_com_funcs;

  pipeline.nb_stages++;

#ifdef RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vrc;
  stages[pipeline.nb_stages].funcs   = video_recorder_funcs;

  pipeline.nb_stages++;
#endif // RECORD_VIDEO
  stages[pipeline.nb_stages].type    = VP_API_FILTER_DECODER;
  stages[pipeline.nb_stages].cfg     = (void*)&vec;
  stages[pipeline.nb_stages].funcs   = vlib_decoding_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_FILTER_YUV2RGB;
  stages[pipeline.nb_stages].cfg     = (void*)&yuv2rgbconf;
  stages[pipeline.nb_stages].funcs   = vp_stages_yuv2rgb_funcs;

  pipeline.nb_stages++;

  stages[pipeline.nb_stages].type    = VP_API_OUTPUT_SDL;
  stages[pipeline.nb_stages].cfg     = NULL;
  stages[pipeline.nb_stages].funcs   = vp_stages_output_gtk_funcs;

  pipeline.nb_stages++;

  pipeline.stages = &stages[0];
 
  /* Processing of a pipeline */
  if( !ardrone_tool_exit() )
  {
    PRINT("Video stage thread initialization\n");

    res = vp_api_open(&pipeline, &pipeline_handle);
   // PRINT("VP_API OPENED\n");
    if( SUCCEED(res) )
    {
      int loop = SUCCESS;
      out.status = VP_API_STATUS_PROCESSING;
      cvStartWindowThread();
      #define frontWindow "DroneView"
      cvNamedWindow(frontWindow, CV_WINDOW_AUTOSIZE);
      frontImgStream = cvCreateImage(cvSize(picture.width, picture.height), IPL_DEPTH_8U, 3);

      #define bottomWindow "BomberView"
      if(extractBottomImage)
	cvNamedWindow(bottomWindow, CV_WINDOW_AUTOSIZE);
      bottomImgStream = cvCreateImage(cvSize(bottomWidth, bottomHeight), IPL_DEPTH_8U, 3);

      
      IplImage *frame;
      CvCapture *capture;
      if(USEWEBCAM)
      {
        capture = cvCaptureFromCAM(WEBCAMINDEX);
    
        if(!capture)
        {
          printf("ERROR: Cannot Initialize Webcam.\n");
          loop = !SUCCESS;
        }
      }

      while( !ardrone_tool_exit() && (loop == SUCCESS) )
      {
        if(!USEWEBCAM)
        {
          if( SUCCEED(vp_api_run(&pipeline, &out)) ) {
              if (vec.controller.num_frames==0) continue;
	      /*int i;
              for(i = 0; i < (picture.width)*(picture.height); i++)
              {
                frontImgStream->imageData[i*3] = picture.y_buf[i];
                frontImgStream->imageData[i*3+1] = picture.y_buf[i];
                frontImgStream->imageData[i*3+2] = picture.y_buf[i];
              }
	      */
             cvCvtColor(rgbHeader, frontImgStream, CV_RGB2BGR); //[for colour]

              if(extractBottomImage)
              {
                int j = 0, i;
                for(i = 0; j < bottomHeight*bottomWidth; i = i%picture.width >= bottomWidth-1 ? i - (i%picture.width) + picture.width : i+1)
                {
                  bottomImgStream->imageData[j*3] = frontImgStream->imageData[i*3];
                  bottomImgStream->imageData[j*3+1] = frontImgStream->imageData[i*3+1];
                  bottomImgStream->imageData[j*3+2] = frontImgStream->imageData[i*3+2];
                  frontImgStream->imageData[i*3] = 0;
                  frontImgStream->imageData[i*3+1] = 0;
                  frontImgStream->imageData[i*3+2] = 0;
                  j++;
                }
              }
              
              //cvLine(frontImgStream, cvPoint(picture.width/2, 0), cvPoint(picture.width/2, picture.height), CV_RGB(0,255,0), 1, CV_AA, 0 );
              cvShowImage(frontWindow, frontImgStream);
              if(extractBottomImage)
		cvShowImage(bottomWindow, bottomImgStream);
              
              if(CAPTUREIMAGESTREAM)
              {
                char filename[256];
                struct timeval t;
                gettimeofday(&t, NULL);
                sprintf(filename, "%d.%06d.jpg", (int)t.tv_sec, (int)t.tv_usec);
                if(frontImgStream != NULL && cvSaveImage(filename, cvCloneImage(frontImgStream), 0))
                  printf("Image dumped to %s\n", filename);
                else
                  printf("Error dumping image...\n");
                if(extractBottomImage)
                {
                  sprintf(filename, "%d.%06dbottom.jpg", (int)t.tv_sec, (int)t.tv_usec);
                  if(bottomImgStream != NULL && cvSaveImage(filename, cvCloneImage(bottomImgStream), 0))
                    printf("Bottom Image dumped to %s\n", filename);
                  else
                    printf("Error dumping bottom image...\n");
                }
              }
          }
          else loop = -1;
        }
        else //use webcam
        {
          frame = cvQueryFrame(capture);
      
          if(!frame) break;
          
          cvResize(frame, frontImgStream, CV_INTER_LINEAR);
          cvShowImage(frontWindow, frontImgStream);
          
          cvWaitKey(1);
        }
      }

      cvDestroyWindow(frontWindow);
      if(extractBottomImage)
	cvDestroyWindow(bottomWindow);
      //cvReleaseImage(&imgBottom);
      //cvDestroyWindow(bottomWindow);
      cvReleaseImage(&frontImgStream);
      vp_api_close(&pipeline, &pipeline_handle);
    }
  }

  PRINT("   Video stage thread ended\n\n");

  return (THREAD_RET)0;
}

