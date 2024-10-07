import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import org.json.simple.JSONObject;

public class CEP_COMMS{

    JSONObject message;
    String ESPAdress;
    int port;
    boolean running = true;

    CEP_COMMS(JSONObject message, String ESPAdress, int port){
        this.message = message;
        this.ESPAdress = ESPAdress;
        this.port = port;
    }

    public void writeToESP32(boolean ACK){
        while(running){
            try {
                if(ACK){
                    this.message = new JSONObject();
                    this.message.put("message", "ACK");
                }else{
                    wait(2000);
                }
                System.out.println(message);
                DatagramSocket socket = new DatagramSocket();
                byte[] buffer = message.toString().getBytes();
                InetAddress address = InetAddress.getByName(ESPAdress);
                DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, port);
                socket.send(packet);
                System.out.println("Message sent to " + ESPAdress + " on port " + port);
                socket.close();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void getFromESP32(){
        try{
            DatagramSocket socket = new DatagramSocket();
            byte[] receiveBuffer = new byte[1024];
            while (true) {
                DatagramPacket packet = new DatagramPacket(receiveBuffer, receiveBuffer.length);
                socket.receive(packet);
                String receivedData = packet.getData().toString();
                System.out.println(receivedData);
                ACKESP32();
            }
        }catch(Exception e){
            e.printStackTrace();
        }
    }

    public void ACKESP32(){
        writeToESP32(true);
    }
    }


