package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {

        // Not right args
        if (args.length == 0) {
            System.out.println("Usage: StompServer <port> <TypeServer>");
            System.exit(1);
        }

        
        // start the server with the thread-per-client server
        if (args[1].equals("tpc")) {
            Server.threadPerClient(
                    Integer.parseInt(args[0]), // port
                    () -> new StompMessagingProtocolImp(), // protocol factory
                    StompMessageEncoderDecoder::new // message encoder decoder factory
            ).serve();
        
        }
        // Start the server with the reactor 
        else if (args[1].equals("reactor")) {
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    Integer.parseInt(args[0]), // port
                    () -> new StompMessagingProtocolImp(), // protocol factory
                    StompMessageEncoderDecoder::new // message encoder decoder factory
            ).serve();
        } else {
            System.out.println("Usage: StompServer <port> <TypeServer>");
            System.exit(1);
        }
    }
}
