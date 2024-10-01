import java.net.InetAddress;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import org.json.simple.JSONObject;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;



public class MCP_COMMS{
    public JSONObject getDataFromMCP() throws IOException{
        //needs to establish connection to the MCP and get the data
        int port = 1;//needs to be changed to real port
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
    }
}





