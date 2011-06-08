package Networking;

public interface MessageListenerInterface {
	void receivedNewMessage(LIMessage msg);   // Will be called when there is a new message ready
}
