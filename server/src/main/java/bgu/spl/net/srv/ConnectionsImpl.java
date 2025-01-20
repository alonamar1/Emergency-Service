package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    private static ConnectionsImpl instance = null;

    private Map<Integer, String> connectionIdToUsername; // connectionId -> username

    private Map<String , User<T>> users; // username -> User

    private Map<String, Map<Integer,User<T>>> channelsSubscribers; // channel -> SubscriptionId -> Users

    private int messageId;

    // TODO: האם צריך להוסיף רשימה של ID ולאיזה צ'אנל הוא רשום
    // TODO: לסנכרן ולהפוך threadsafe
    // TODO: server type
    
    private ConnectionsImpl() {
        this.users = new HashMap<>();
        this.channelsSubscribers = new HashMap<>();
        this.connectionIdToUsername = new HashMap<>();
        this.messageId = 1;
    }

    public static synchronized ConnectionsImpl getInstance() {
        if (instance == null) {
            instance = new ConnectionsImpl();
        }
        return instance;
    }

    @Override
    public boolean send(int connectionId, T msg) {
        if (users.containsKey(connectionIdToUsername.get(connectionId))) {
            users.get(connectionIdToUsername.get(connectionId)).GetConnectionHandler().send(msg);
            this.messageId++;
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        if (channelsSubscribers.containsKey(channel)) {
            Map<Integer, User<T>> subscribers = channelsSubscribers.get(channel);
            for (int connectionId : subscribers.keySet()) {
                send(connectionId, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        User user = users.get(connectionIdToUsername.get(connectionId));
        Map<Integer, String>  usersChannels = user.GetChannels();
        for (Map.Entry<Integer, String> entry : usersChannels.entrySet())
        {
            channelsSubscribers.get(entry.getValue()).remove(entry.getKey());
        }
        user.Disconnect();
        users.remove(connectionIdToUsername.get(connectionId));
        
    }

    public int getMessageID()
    {
        return this.messageId;
    }

    public Map<String , User<T>> getUsers()
    {
        return users;
    }

    public Map<String, Map<Integer,User<T>>> getChannelsSubscribers()
    {
        return channelsSubscribers;
    }

    public void addSubscriber(String channel, int subscriptionId, User<T> user)
    {
        if (!channelsSubscribers.containsKey(channel))
        {
            channelsSubscribers.put(channel, new HashMap<>());
        }
        channelsSubscribers.get(channel).put(subscriptionId, user);
    }

    public Map<Integer, String> getConnectionIdToUsernam()
    {
        return connectionIdToUsername;
    }

    public void removeSubscriber(String channel, int subscriptionId)
    {
        if (channelsSubscribers.containsKey(channel))
        {
            channelsSubscribers.get(channel).remove(subscriptionId);
        }
    }

    public User<T> getUser(int connectionId)
    {
        return users.get(connectionIdToUsername.get(connectionId));
    }

}
