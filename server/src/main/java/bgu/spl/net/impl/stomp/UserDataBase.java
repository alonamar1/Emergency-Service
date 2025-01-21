package bgu.spl.net.impl.stomp;

import java.util.concurrent.ConcurrentHashMap;

public class UserDataBase {

    private static class UserDataBaseHolder {
        private static UserDataBase instance = new UserDataBase();

    }

    // username -> User
    // save every user that connect to the system
    private ConcurrentHashMap<String, User<String>> users;

    private UserDataBase() {
        users = new ConcurrentHashMap<>();
    }

    public static UserDataBase getInstance() {
        return UserDataBaseHolder.instance;
    }

    /**
     * add user to the system
     * @param name
     * @param user
     * @return if success return true, else return false
     */
    public boolean addUser(String name, User<String> user) {
        synchronized (users) {
            if (users.containsKey(name)) {
                return false;
            }
            users.put(name, user);
            return true;
        }
    }

    /**
     * get user from the system
     * @param name
     * @return
     */
    public User<String> getUser(String name) {
        return users.get(name);
    }

    public ConcurrentHashMap<String, User<String>> getUsers() {
        return users;
    }

    /**
     * check if the user is in the system
     * @param name
     * @return
     */
    public boolean containsUser(String name) {
        return users.containsKey(name);
    }

    /**
     * remove user from the system
     * @param name
     * @return
     */
    public boolean removeUser(String name) {
        synchronized (users) { 
            if (users.containsKey(name)) {
                users.remove(name);
                return true;
            }
            return false;
        }
    }

}

