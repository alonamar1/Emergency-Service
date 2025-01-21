package bgu.spl.net.impl.stomp;

import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class Frame {

    private List<String> headers;
    private List<String> body;
    private String type;
    private ConnectionsImpl connections;
    private int connectionId;

    public Frame(String message, ConnectionsImpl connection, int conID) {
        headers = new LinkedList<>();
        body = new LinkedList<>();
        String[] lines = message.split("\n");
        this.type = lines[0];
        int i = 1;
        while (i < lines.length && !lines[i].equals("")) {
            this.headers.add(lines[i]);
            i++;
        }
        i++;
        while (i < lines.length) {
            this.body.add(lines[i]);
            i++;
        }
        this.connections = connection;
        this.connectionId = conID;
    }

    public List<String> getHeader() {
        return headers;
    }

    public List<String> getBody() {
        return body;
    }

    public String getType() {
        return type;
    }

    public void process() {
        // TODO: add recipt header to all the frames
        switch (this.type) {
            case "CONNECT":
                processConnect();
                break;
            case "SEND":
                processSend();
                break;
            case "SUBSCRIBE":
                processSubsribe();
                break;
            case "UNSUBSCRIBE":
                processUnsubsribe();
                break;
            case "DISCONNECT":
                processDisconnect();
                break;
            default:
                //TODO: what is the process here?
                throw new AssertionError("UNVALID FRAME TYPE");
        }
    }

    public void processConnect() {
        boolean exits = false;
        boolean connected = false;
        String username = "";
        String password = "";
        User<String> user = null;

        for (String line : headers) {
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
                username = parts[1];

            } else if (parts[0].equals("passcode")) {
                password = parts[1];
            }
        }
        exits = connections.getUsers().containsKey(username);
        // check if the user is exits
        if (exits) {
            user = connections.getUserByName(username);
            connected = user.IsConnected();

            // if the user allready connected or the password is wrong sent an error frame
            if (connected) {
                // TODO: sent an error frame
            }
            if (!user.CheckPassword(password)) {
                // TODO: sent an error frame
            }
            
            user.Connect(connectionId, connections.GetConnectionHandler(connectionId));
            connections.addUserConnections(connectionId, username, user);
        }

        if (!exits) {
            user = new User<String>(username, password, this.connectionId,
                    connections.GetConnectionHandler(this.connectionId));
            connections.addUserConnections(this.connectionId, username, user);
        }
        System.out.println(username + " CONNECTED");
        user.Connect(this.connectionId, connections.GetConnectionHandler(this.connectionId));

        connections.send(this.connectionId, "CONNECTED\nversion:1.2\n\n");
    }

    public void processSubsribe() {
        // TODO: sent an error frame
        int subscriptionId = -1;
        String destination = "";
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("destination")) {
                destination = parts[1];
            } else if (parts[0].equals("id")) {
                subscriptionId = Integer.parseInt(parts[1]);
            }
        }
        String username = connections.getConnectionIdToUsernam().get(connectionId);
        System.out.println( username + " SUBSCRIBE to " + destination + " with id " + subscriptionId);
        connections.addSubscriber(destination, subscriptionId, connections.getUserByName(username));
        // TODO: sent a receipt frame
        connections.send(this.connectionId, "RECEIPT\nreceipt-id:1\n\n");
    }

    public void processUnsubsribe() {
        int subscriptionId = -1;
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("id")) {
                subscriptionId = Integer.parseInt(parts[1]);
            }
        }
        String username = (String) connections.getConnectionIdToUsernam().get(connectionId);
        
        User<String> user = connections.getUsers().get(username);
        String channel = user.GetChannels().get(subscriptionId);
        System.out.println(username + " UNSUBSCRIBE from " + channel + " with id " + Integer.toString(subscriptionId));
        connections.removeSubscriber(channel, subscriptionId);
        // TODO: sent a receipt frame
        connections.send(this.connectionId, "RECEIPT\nreceipt-id:1\n\n");
        // TODO: sent an error frame
    }

    public void processDisconnect() {
        // TODO: sent an error frame
        int reciptId = -1;
        String username = (String) connections.getConnectionIdToUsernam().get(connectionId);
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("receipt")) {
                reciptId = Integer.parseInt(parts[1]);
            }
        }
        // Send the message
        connections.send(this.connectionId, "RECEIPT\nreceipt-id:" + reciptId + "\n\n");
        // Then disconnect the user
        connections.disconnect(this.connectionId);
        System.out.println(username + " DISCONNECTED with recipt " + reciptId);
    }

    public void processSend() {
        String dest = "";
        String bodyMessage = "";
        String message = "";

        // construct the body messsage
        for (String line : body) {
            bodyMessage += line + "\n";
        }

        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("destination")) {
                dest = parts[1];
            }
        }
        // TODO: need to delete the sunstring function -  באמת לא צריך אותה
        List<User<String>> usersToSend = this.connections.getChannelsSubscribers().get(dest.substring(1));
        System.out.println("SEND to " + dest);
        for (User<String> user : usersToSend) {
            message = "MESSAGE\nsubsription:" + user.getIdSubscription(dest) + "\nmessage-id:" + connections.getMessageID()
                    + "\ndestination:" + dest + "\n\n" + bodyMessage;
            connections.send(user.GetConnectionId(), message);
        }
    }

    // public static void main(String[] args) {
    // Frame f = new
    // Frame("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:admin\npasscode:admin\n\nranivgiAluf\u0000",
    // null, 1);
    // System.out.println(f.getType().toString());
    // System.out.println(f.getHeader().toString());
    // System.out.println(f.getBody().toString());
    // }

}
