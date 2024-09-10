import java.net.InetAddress;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

import org.json.simple.JSONObject;


public class MCP_COMMS_thread extends COMMS_thread {
    int port ;
    public JSONObject getDataFromMCP(){
       ByteBuffer buffer = ByteBuffer.allocate(100);
        return null;
    };  

    try {
        // Create a DatagramSocket to listen on the specified port
        DatagramSocket socket = new DatagramSocket(port);

        // Create a buffer to store incoming data
        byte[] receiveBuffer = new byte[1024];

        System.out.println("Listening on port " + port + "...");

        while (true) {
            // Prepare a DatagramPacket to receive data
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length);

            // Receive data from the sender
            socket.receive(receivePacket);

            // Convert the received bytes into a string
            String receivedData = new String(receivePacket.getData(), 0, receivePacket.getLength());

            // Print the received data
            System.out.println("Received: " + receivedData);

            // Optionally, process the data or break the loop if needed
        }
    } catch (Exception e) {
        e.printStackTrace();
    }
}





