package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.stomp.User;
import bgu.spl.net.impl.stomp.UserDataBase;

public class ConnectionsImpl implements Connections<String> {

    private static class ConnectionHandlerHolder {
        private static ConnectionsImpl instance = new ConnectionsImpl();
    }

    private UserDataBase userDataBase; // hold all users, active and none active
    private Map<Integer, ConnectionHandler<String>> connectionIdToconnectionHandler; // ConnectionHandler ->
                                                                                     // connectionId
    private Map<Integer, String> connectionIdToUsername; // connectionId -> username
    private Map<String, Map<Integer, User<String>>> channelsSubscribers; // channel -> SubscriptionId -> Users
    private AtomicInteger messageId;

    // TODO: server type

    private ConnectionsImpl() {
        this.channelsSubscribers = new ConcurrentHashMap<>();
        this.connectionIdToUsername = new ConcurrentHashMap<>();
        this.connectionIdToconnectionHandler = new ConcurrentHashMap<>();
        this.userDataBase = UserDataBase.getInstance();
        this.messageId = new AtomicInteger(1);
    }

    public static ConnectionsImpl getInstance() {
        return ConnectionHandlerHolder.instance;
    }

    @Override
    public boolean send(int connectionId, String msg) {
        synchronized (connectionIdToconnectionHandler) {
            ConnectionHandler<String> handler = connectionIdToconnectionHandler.get(connectionId);
            if (handler == null) {
                return false;
            }
            handler.send(msg);
            this.messageId.incrementAndGet();
            return true;
        }
    }

    @Override
    public void send(String channel, String msg) {
        synchronized (channelsSubscribers) {
            Map<Integer, User<String>> subscribers = channelsSubscribers.get(channel);
            if (subscribers != null) {
                for (Map.Entry<Integer, User<String>> entry : subscribers.entrySet()) {
                    this.send(entry.getValue().GetConnectionId(), msg);
                }
            }
        }
    }

    @Override
    public synchronized void disconnect(int connectionId) {
        User<String> user = this.getUser(connectionId);
        Map<Integer, String> usersChannels = user.GetChannels();
        for (Map.Entry<Integer, String> entry : usersChannels.entrySet()) {
            channelsSubscribers.get(entry.getValue()).remove(entry.getKey());
        }
        // disconnect the user
        user.Disconnect();
        // delete from id to username
        connectionIdToUsername.remove(connectionId);
        // delete from the id to connection handler
        connectionIdToconnectionHandler.remove(connectionId);

    }

    public void addConnectionHandler(int connectionId, ConnectionHandler handler) {
        this.connectionIdToconnectionHandler.put(connectionId, handler);
    }

    /**
     * add user to the system
     * 
     * @param connectid
     * @param username
     * @param user
     */
    public void addUserConnections(int connectid, String username, User<String> user) {
        userDataBase.addUser(username, user);
        connectionIdToUsername.put(connectid, username);
    }

    /**
     * get the connection handler by the connection id
     * 
     * @param connectionId
     * @return
     */
    public ConnectionHandler<String> GetConnectionHandler(int connectionId) {
        return this.connectionIdToconnectionHandler.get(connectionId);
    }

    public int getMessageID() {
        return this.messageId.get();
    }

    public Map<String, User<String>> getUsers() {
        return userDataBase.getUsers();
    }

    public Map<String, Map<Integer, User<String>>> getChannelsSubscribers() {
        return channelsSubscribers;
    }

    public void addSubscriber(String channel, int subscriptionId, User<String> user) {
        synchronized (channelsSubscribers) {
            if (!channelsSubscribers.containsKey(channel)) {
                channelsSubscribers.put(channel, new ConcurrentHashMap<>());
            }
            // add the subscription id to the user
            user.addSubscriptionIdInChannel(channel, subscriptionId);
            // add the user to the channel
            channelsSubscribers.get(channel).put(subscriptionId, user);
        }
    }

    public Map<Integer, String> getConnectionIdToUsernam() {
        return connectionIdToUsername;
    }

    public void removeSubscriber(String channel, int subscriptionId) {
        synchronized (channelsSubscribers) {
            if (channelsSubscribers.containsKey(channel)) {
                channelsSubscribers.get(channel).remove(subscriptionId);
            }
        }
    }

    public User<String> getUser(int connectionId) {
        synchronized (connectionIdToUsername) {
            return userDataBase.getUser(connectionIdToUsername.get(connectionId));
        }
    }

    public User<String> getUserByName(String username) {
        return userDataBase.getUser(username);
    }

}
