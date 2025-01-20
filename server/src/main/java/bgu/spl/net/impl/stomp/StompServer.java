package bgu.spl.net.impl.stomp;

import bgu.spl.net.impl.echo.EchoProtocol;
import bgu.spl.net.impl.echo.LineMessageEncoderDecoder;
import bgu.spl.net.srv.Server;
import bgu.spl.net.impl.stomp.StompMessageEncoderDecoder;

public class StompServer {

    public static void main(String[] args) {
        // TODO: implement this

        // you can use any server...
        Server.threadPerClient(
                7777, // port
                () -> new StompMessagingProtocolImp(), // protocol factory
                StompMessageEncoderDecoder::new // message encoder decoder factory
        ).serve();

        // Server.reactor(
        // Runtime.getRuntime().availableProcessors(),
        // 7777, //port
        // () -> new EchoProtocol<>(), //protocol factory
        // LineMessageEncoderDecoder::new //message encoder decoder factory
        // ).serve();
    }
}
