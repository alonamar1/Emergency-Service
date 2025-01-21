package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImp implements StompMessagingProtocol<String> {

    private Connections<String> connections;
    private int connectionId;
    private boolean shouldTerminate;

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connections = connections;
        this.connectionId = connectionId;
        this.shouldTerminate = false;
    }

    @Override
    public void process(String message) {
        // TODO: fix the first user connect frame
        Frame frame = new Frame(message, (ConnectionsImpl) connections, connectionId);
        frame.process();
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

}
