package MessageComponent;

public interface MessageListenerInterface {
	void receivedNewMessage(LIMessage msg);	// Will be called when there is a new message ready
	void connectionStatus(boolean bool);	// Method called whenever the status of the connection changes
}
