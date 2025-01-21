package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.srv.ConnectionHandler;

public class User<T> {

    private String username;
    private String password;
    private boolean isConnected;
    private int connectionId;
    private Map<Integer, String> channels;
    private ConnectionHandler<T> connectionHandler;

    public User(String username, String password, int connectionId, ConnectionHandler<T> connectionHandler) {
        this.username = username;
        this.password = password;
        this.isConnected = true;
        this.connectionId = connectionId;
        this.channels = new HashMap<>();
        this.connectionHandler = connectionHandler;
    }

    public boolean CheckUsername(String username) {
        return this.username.equals(username);
    }

    public boolean CheckPassword(String password) {
        return this.password.equals(password);
    }

    public void Connect(int connectionId, ConnectionHandler<T> connectionHandler) {
        this.isConnected = true;
        this.connectionId = connectionId;
        this.connectionHandler = connectionHandler;
    }

    public void Disconnect() {
        this.isConnected = false;
        this.connectionId = -1;
        this.connectionHandler = null;
    }

    public boolean IsConnected() {
        return this.isConnected;
    }

    public int GetConnectionId() {
        return this.connectionId;
    }

    public String GetUsername() {
        return this.username;
    }

    public ConnectionHandler<T> GetConnectionHandler() {
        return this.connectionHandler;
    }

    public Map<Integer, String> GetChannels() {
        return this.channels;
    }


    /**
     * Add a connection to a channel
     * @param channel
     * @return null if the user is not registered to the channel, otherwise return the Id for the channel
     */
    public String GetSubscriptionIdInChannel(int subscriptionId) {
        return this.channels.get(subscriptionId);
    }

    public void addSubscriptionIdInChannel(String channel, int subscriptionId) {
         this.channels.put(subscriptionId, channel);
    }

    public void removeSubscriptionIdInChannel(int subscriptionId) {
        this.channels.remove(subscriptionId);
    }
}
