package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final MessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private int connectionId;

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, MessagingProtocol<T> protocol, int conectID) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        this.connectionId = conectID;
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { // just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            // initialize the protocol
            protocol.start(this.connectionId, (Connections<T>) ConnectionsImpl.getInstance());
            
            // add this connection to the connections
            ConnectionsImpl.getInstance().addConnectionHandler(this.connectionId, this);

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null) {
                    protocol.process(nextMessage);
                    // if (response != null) {
                    //     out.write(encdec.encode(response));
                    //     out.flush();
                    // }
                }
            }
            // if the user is still connected for somne reason, the socket close unexpected, disconnect him
            if (ConnectionsImpl.getInstance().getUser(this.connectionId).IsConnected()) {
                ConnectionsImpl.getInstance().disconnect(this.connectionId);
            }
            this.close();

        } catch (IOException ex) {
            ex.printStackTrace();
        }

    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg) {
        // send the message in Thread per client protocol
        try {
            if (msg != null) {
                out.write(encdec.encode(msg));
                out.flush();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
