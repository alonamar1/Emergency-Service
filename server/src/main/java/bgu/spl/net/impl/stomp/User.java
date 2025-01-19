package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.srv.ConnectionHandler;

public class User<T> {

    private String username;
    private String password;
    private boolean isConnected;
    private int connectionId;
    private Map<String, Integer> channels;
    private ConnectionHandler<T> connectionHandler;

    public User(String username, String password) {
        this.username = username;
        this.password = password;
        this.isConnected = false;
        this.connectionId = -1;
        this.channels = new HashMap<>();
        this.connectionHandler = null;
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


    /**
     * Add a connection to a channel
     * @param channel
     * @return null if the user is not registered to the channel, otherwise return the Id for the channel
     */
    public int GetConnectionIdInChannel(String channel) {
        return this.channels.get(channel);
    }
    

}
