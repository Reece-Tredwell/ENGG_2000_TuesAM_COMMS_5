
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import org.json.simple.JSONObject;

public class CEP_COMMS_thread{

    JSONObject message;
    String ESPAdress;  // or use the server's IP address
    int port;

    CEP_COMMS_thread(JSONObject message, String ESPAdress, int port){
        this.message = message;
        this.ESPAdress = ESPAdress;  // oSr use the server's IP address
        this.port = port;
    }

    //Gets from buffer
    public void writeToESP32(){
        try {
            System.out.println(message);
            // Create a DatagramSocket
            DatagramSocket socket = new DatagramSocket();

            // Convert the message to bytes
            byte[] buffer = message.toString().getBytes();

            // Get the server's address (IP)
            InetAddress address = InetAddress.getByName(ESPAdress);

            // Create a DatagramPacket to send the data
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, port);

            // Send the packet
            socket.send(packet);

            System.out.println("Message sent to " + ESPAdress + " on port " + port);

            // Close the socket after sending
            socket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

