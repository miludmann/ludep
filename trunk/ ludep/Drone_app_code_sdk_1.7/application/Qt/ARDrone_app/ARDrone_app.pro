#-------------------------------------------------
#
# Project created by QtCreator 2011-05-26T10:49:06
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = ARDrone_app
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += link_pgkconfig
PGKCONFIG += opencv


TEMPLATE = app


SOURCES += \
    UI/planner.cpp \
    UI/ui.c \
    UI/planner_BKP.cpp \
    UI/keyboard.cpp \
    UI/gamepad.cpp \
    ardrone_testing_tool.c \
    Tools/smoothingmedianfilter.cpp \
    Tools/medianfilter.cpp \
    Tools/coopertools.cpp \
    Navdata/NavDataContainer.cpp \
    Navdata/navdata.c \
    Video/video_stage.c

HEADERS += \
    UI/planner.hpp \
    UI/ui.h \
    UI/terminalinput.hpp \
    UI/keyboard.h \
    UI/gamepad.h \
    ardrone_testing_tool.h \
    Tools/smoothingmedianfilter.hpp \
    Tools/medianfilter.hpp \
    Tools/cvpointtools.hpp \
    Tools/coopertools.hpp \
    Navdata/NavDataContainer.hpp~ \
    Navdata/NavDataContainer.hpp \
    Navdata/NavDataContainer.cpp~ \
    Navdata/navdata.h \
    Video/video_stage.h \
    Video/video_stage.c~
