package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImp implements StompMessagingProtocol<String> {

    private Connections<String> connections;
    private int connectionId;
    private boolean shouldTerminate;

    public StompMessagingProtocolImp(int connectionId, Connections<String> connections) {
        this.start(connectionId, connections);
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connections = connections;
        this.connectionId = connectionId;
        this.shouldTerminate = false;
    }

    @Override
    public void process(String message) {
        // TODO: sent an error frame if the message is not valid
        String[] lines = message.split("\n");
        String typeFrame = lines[0];
        switch (typeFrame) {
            case "CONNECT": {
                for (String line : lines) {
                    String[] parts = line.split(":");
                    if (parts[0].equals("accept-version")) {
                        if (!parts[1].equals("1.2")) {
                            // TODO: sent an error frame
                        }
                    } else if (parts[0].equals("host")) {
                        if (!parts[1].equals("stomp.cs.bgu.ac.il")) {
                            // TODO: sent an error frame
                        }
                    } else if (parts[0].equals("login")) {
                        // TODO: check if the login is valid

                    } else if (parts[0].equals("passcode")) {
                        // TODO: check if the passcode is valid

                    }
                }
                this.connections.send(this.connectionId, "CONNECTED\nversion:1.2\n\n\u0000");
                break;
            }
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
