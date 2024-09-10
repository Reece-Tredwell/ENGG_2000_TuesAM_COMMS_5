
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class CEP_COMMS_thread{

    String message;
    String serverAddress;  // or use the server's IP address
    int port;

    CEP_COMMS_thread(String message, String serverAddress, int port){
        message = this.message;
        serverAddress = this.serverAddress;  // or use the server's IP address
        port = this.port;
    }

    //Gets from buffer
    public void writeToESP32(){
        try {
            // Create a DatagramSocket
            DatagramSocket socket = new DatagramSocket();

            // Convert the message to bytes
            byte[] buffer = message.getBytes();

            // Get the server's address (IP)
            InetAddress address = InetAddress.getByName(serverAddress);

            // Create a DatagramPacket to send the data
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, port);

            // Send the packet
            socket.send(packet);

            System.out.println("Message sent to " + serverAddress + " on port " + port);

            // Close the socket after sending
            socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

