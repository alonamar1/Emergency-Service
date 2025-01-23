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

    /**
     * process the Message
     */
    public void process() {
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
                String errorMesg = createErrorMessage(this.type + "\n\n", "Unknown frame type", -1, null);
                // close the socket in ConnectionHandler and send meesage to the user
                this.handleErrorSendAndDisconnect(errorMesg);
                // remove the connection handle from the connections
                connections.getconnectionIdToconnectionHandler().remove(this.connectionId);
                // Stop the function
                return;
        }
    }

    /**
     * process the CONNECT frame
     */
    public void processConnect() {
        boolean exits = false;
        boolean connected = false;
        String username = "";
        String password = "";
        String host = "";
        String acceptVersion = "";
        User<String> user = null;

        // get the headers
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("accept-version")) {
                acceptVersion = parts[1];
            } else if (parts[0].equals("host")) {
                host = parts[1];
            } else if (parts[0].equals("login")) {
                username = parts[1];
            } else if (parts[0].equals("passcode")) {
                password = parts[1];
            }
        }

        // check if the host and version are correct
        if (!host.equals("stomp.cs.bgu.ac.il") || !acceptVersion.equals("1.2")) {
            String errorMesg = createErrorMessage("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:"
                    + username + "\npasscode:" + password + "\n\n", "Wrong host or version", -1,
                    "We support only version: 1.2 AND host: stomp.cs.bgu.ac.il");
            // close the socket in ConnectionHandler and send meesage to the user
            this.handleErrorSendAndDisconnect(errorMesg);
            // remove the connection handle from the connections
            connections.getconnectionIdToconnectionHandler().remove(this.connectionId);
            // Stop the function
            return;
        }

        exits = connections.getUsers().containsKey(username);
        // check if the user is exits
        if (exits) {
            user = connections.getUserByName(username);
            connected = user.IsConnected();

            // TODO: check in case of reactor if need to disconnect the socket from here

            // if the user allready connected or the password is wrong sent an error frame
            if (connected) {
                String errorMesg = createErrorMessage("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:"
                        + username + "\npasscode:" + password + "\n\n", "User already logged in", -1, null);
                // close the socket in ConnectionHandler and send meesage to the user
                this.handleErrorSendAndDisconnect(errorMesg);
                // remove the connection handle from the connections
                connections.getconnectionIdToconnectionHandler().remove(this.connectionId);
                // Stop the function
                return;
            }
            if (!user.CheckPassword(password)) {
                String errorMesg = createErrorMessage("CONNECT\naccept-version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:"
                        + username + "\npasscode:" + password + "\n\n", "Wrong password", -1, null);
                // close the socket in ConnectionHandler and send meesage to the user
                this.handleErrorSendAndDisconnect(errorMesg);
                // remove the connection handle from the connections
                connections.getconnectionIdToconnectionHandler().remove(this.connectionId);
                // Stop the function
                return;
            }
            // if the user is exits and not connected connect the user as needed
            user.Connect(connectionId, connections.GetConnectionHandler(connectionId));
            // connections.addUserConnections(connectionId, username, user);
        }

        // if the user not exits create a new user
        if (!exits) {
            user = new User<String>(username, password, this.connectionId,
                    connections.GetConnectionHandler(this.connectionId));
            // connections.addUserConnections(this.connectionId, username, user);
        }
        System.out.println(username + " CONNECTED");
        // add the user to connected users
        connections.addUserConnections(connectionId, username, user);
        // append the connection handler and connect signals to the user object
        user.Connect(this.connectionId, connections.GetConnectionHandler(this.connectionId));
        // send the connected message to the user
        connections.send(this.connectionId, "CONNECTED\nversion:1.2\n\n");
    }

    /**
     * process the SUBSCRIBE frame
     */
    public void processSubsribe() {
        // TODO: error if there is no dest\id, handle in the client side!
        // TODO: handle the case where the client sent dest as " " or ""
        int subscriptionId = -1;
        String destination = "";
        int reciptId = -1;
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("destination")) {
                destination = parts[1];
            } else if (parts[0].equals("id")) {
                subscriptionId = Integer.parseInt(parts[1]);
            } else if (parts[0].equals("receipt")) {
                reciptId = Integer.parseInt(parts[1]);
            }
        }
        String username = connections.getConnectionIdToUsernam().get(connectionId);
        System.out.println(
                username + " SUBSCRIBE to " + destination + " with id " + subscriptionId + " with recipt " + reciptId);
        // try add the user to the channel
        try {
            connections.addSubscriber(destination, subscriptionId, connections.getUserByName(username));
        } catch (Exception e) {
            String errorMesg = createErrorMessage("SUBSCRIBE\ndestination:" + destination + "\nid:" + subscriptionId
                    + "\n\n", "Subscribe function failed", reciptId, null);
            // close the socket in ConnectionHandler and send the message to the user
            this.handleErrorSendAndDisconnect(errorMesg);
            // disconnect the user
            connections.disconnect(this.connectionId);
            return;
        }
        connections.send(this.connectionId, "RECEIPT\nreceipt-id:" + reciptId + "\n\n");
    }

    /**
     * process the UNSUBSCRIBE frame
     */
    public void processUnsubsribe() {
        int subscriptionId = -1;
        int reciptId = -1;
        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("id")) {
                subscriptionId = Integer.parseInt(parts[1]);
            } else if (parts[0].equals("receipt")) {
                reciptId = Integer.parseInt(parts[1]);
            }
        }
        // get the username by the connection id
        String username = (String) connections.getConnectionIdToUsernam().get(connectionId);
        // get the user by the username
        User<String> user = connections.getUsers().get(username);
        // get the channel by the subscription id and user
        String channel = user.GetChannels().get(subscriptionId);
        // handle the case that the user not have this subscription id
        if (channel == null || channel.equals("")) {
            String errorMesg = createErrorMessage("UNSUBSCRIBE\nid:" + subscriptionId + "\n\n", "User not subscribed",
                    reciptId, "There is no subscription with id " + Integer.toString(subscriptionId));
            // close the socket in ConnectionHandler and send the message to the user
            this.handleErrorSendAndDisconnect(errorMesg);
            // disconnect the user
            connections.disconnect(this.connectionId);
            return;
        }
        System.out.println(username + " UNSUBSCRIBE from " + channel + " with id " + Integer.toString(subscriptionId)
                + " with recipt " + reciptId);
        connections.removeSubscriber(channel, subscriptionId, this.connectionId);
        connections.send(this.connectionId, "RECEIPT\nreceipt-id:" + reciptId + "\n\n");
        // TODO: error if there is no id, handle in the client side!
    }

    /**
     * process the DISCONNECT frame
     */
    public void processDisconnect() {
        // TODO: handle in client if there is no id recipt
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

    /**
     * process the SEND frame
     */
    public void processSend() {
        String dest = "";
        String bodyMessage = "";
        String message = "";
        int reciptId = -1;

        // construct the body messsage
        for (String line : body) {
            bodyMessage += line + "\n";
        }

        for (String line : headers) {
            String[] parts = line.split(":");
            if (parts[0].equals("destination")) {
                dest = parts[1];
            } else if (parts[0].equals("receipt")) {
                reciptId = Integer.parseInt(parts[1]);
            }
        }
        // TODO: need to delete the sunstring function - באמת לא צריך אותה
        List<User<String>> usersToSend = this.connections.getChannelsSubscribers().get(dest);

        // handle the case that there is no such channel
        // TODO: handle the case where the client no register to the channel \ not sent
        // dest
        if (usersToSend == null) {
            String errorMesg = createErrorMessage("SEND\ndestination:" + dest + "\n\n", "No such channel", reciptId,
                    "There is no such channel " + dest);
            // close the socket in ConnectionHandler and send the message to the user
            this.handleErrorSendAndDisconnect(errorMesg);
            // disconnect the user
            connections.disconnect(this.connectionId);
            return;
        }

        // Try send the message to all the users in the channel
        try {
            for (User<String> user : usersToSend) {
                message = "MESSAGE\nsubsription:" + user.getIdSubscription(dest) + "\nmessage-id:"
                        + connections.getMessageID()
                        + "\ndestination:" + dest + "\n\n" + bodyMessage;
                connections.send(user.GetConnectionId(), message);
            }
        } catch (Exception e) {
            String errorMesg = createErrorMessage("SEND\ndestination:" + dest + "\n\n", "Send function failed",
                    reciptId, null);
            // close the socket in ConnectionHandler and send the message to the user
            this.handleErrorSendAndDisconnect(errorMesg);
            // disconnect the user
            connections.disconnect(this.connectionId);
        }
        System.out.println("SEND to " + dest);
        // Send confirmation message
        // TODO: there is a chance that this line give us bugs with the client they gave us
        // connections.send(this.connectionId, "RECEIPT\nreceipt-id:" + reciptId + "\n\n");
    }

    /**
     * create an error message
     * 
     * @param Frame
     * @param error
     * @param recipt
     * @param details
     * @return the error message
     */
    public String createErrorMessage(String Frame, String error, int recipt, String details) {
        String output = "ERROR\nreceipt-id:" + recipt + "\nmessage:" + error + "\n\n" + "The message:\n" + "-----\n"
                + Frame;
        if (details != null) {
            output += "-----\n" + details;
        }
        return output;
    }

    /**
     * handle the error message and disconnect the user
     * 
     * @param Message
     * @param recipt
     */
    public void handleErrorSendAndDisconnect(String Message) {
        connections.send(this.connectionId, Message);
        // close the socket in ConnectionHandler
        try {
            connections.GetConnectionHandler(this.connectionId).close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        // disconnect the user in ConnectionImpl
        // TODO: check if need to delete something else
        // connections.disconnect(this.connectionId);
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
