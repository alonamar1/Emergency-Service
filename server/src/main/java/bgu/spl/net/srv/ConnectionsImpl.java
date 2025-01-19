package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    private Map<Integer, String> connectionIdToUsername; // connectionId -> username

    private Map<String , User<T>> users; // username -> User

    private Map<String, Map<Integer,Integer>> channelsSubscribers; // channel -> connectionId -> SubscriptionId

    // TODO: האם צריך להוסיף רשימה של ID ולאיזה צ'אנל הוא רשום
    // TODO: לסנכרן ולהפוך threadsafe
    // TODO: server type
    
    public ConnectionsImpl() {
        this.users = new HashMap<>();
        this.channelsSubscribers = new HashMap<>();
        this.connectionIdToUsername = new HashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        if (users.containsKey(connectionIdToUsername.get(connectionId))) {
            users.get(connectionIdToUsername.get(connectionId)).GetConnectionHandler().send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        if (channelsSubscribers.containsKey(channel)) {
            Map<Integer, Integer> subscribers = channelsSubscribers.get(channel);
            for (int connectionId : subscribers.keySet()) {
                send(connectionId, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        users.get(connectionIdToUsername.get(connectionId)).Disconnect();
        users.remove(connectionIdToUsername.get(connectionId));
    }

}
