package bgu.spl.net.api;

import bgu.spl.net.srv.Connections;

public class StompMessagingProtocolImp implements StompMessagingProtocol<String> {

    //private Connections<String> connections;
    private boolean shouldTerminate;

    public StompMessagingProtocolImp() {
        //connections = new ConnectionsImpl<>();
        shouldTerminate = false;
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {

    }

    @Override
    public void process(String message) {
        String[] lines = message.split("\n");
        String typeFrame = lines[0];
        switch (typeFrame) {
            case "CONNECT":
                break;
            case "SEND":
                break;
            case "SUBSRICE":
                break;
            case "UNSUBSRICE":
                break;
            case "DISCONNECT":
                break;
            default: 
                throw new AssertionError("UNVALID FRAME TYPE");
        }

    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

}
