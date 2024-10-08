import org.json.simple.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;



public class MCP_COMMS{
    public JSONObject getDataFromMCP() throws IOException{
        int port = 1;
        DatagramSocket socket = new DatagramSocket(port);
        byte[] receiveBuffer = new byte[1024];
        System.out.println("Listening on port " + port + "...");
        while (true) {
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length);
            socket.receive(receivePacket);
            String receivedData = new String(receivePacket.getData(), 0, receivePacket.getLength());
            System.out.println("Received: " + receivedData);
        }
    }
}





