package bgu.spl.net.srv;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Queue;

public class ConnectionsImpl<T> implements Connections<T> {

    private Map<Integer, Queue<T>> clients;
    private Map<String, List<Integer>> channelsSubscribers;
    // TODO: Add a lock for each map
    // TODO: האם צריך להוסיף רשימה של ID ולאיזה צ'אנל הוא רשום
    
    public ConnectionsImpl() {
        this.clients = new HashMap<>();
        this.channelsSubscribers = new HashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        if (clients.containsKey(connectionId)) {
            clients.get(connectionId).add(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        if (channelsSubscribers.containsKey(channel)) {
            for (int connectionId : channelsSubscribers.get(channel)) {
                send(connectionId, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        clients.remove(connectionId);
    }

}
