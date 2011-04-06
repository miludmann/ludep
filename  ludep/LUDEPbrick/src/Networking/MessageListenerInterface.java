package Networking;

public interface MessageListenerInterface {
	void recievedNewMessage(LIMessage msg);   // Will be called when there is a new message ready
}
