import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.Random;

import java.nio.*;
import java.nio.channels.*;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class MockMCP {
    private final String clientType = "CCP";
    private final String clientID = "BR10";

    private DatagramChannel CCPChannel;


    private InetSocketAddress CCPIP;
    private int CCPPort;

    private boolean connected = false;

    private int sequenceNumber;

    long lastHeartbeatTime = 0;


    private JSONObject receiveMessage(DatagramChannel channel) throws Exception {
        ByteBuffer receiveBuffer = ByteBuffer.allocate(2048);
        SocketAddress sender = CCPChannel.receive(receiveBuffer);
        JSONObject message = null;

        if (sender != null) {
            receiveBuffer.flip();
            byte[] receivedData = new byte[receiveBuffer.remaining()];
            receiveBuffer.get(receivedData);
            message = (JSONObject)new JSONParser().parse(new String(receivedData));
            receiveBuffer.clear();
        }

        return message;
    }
    private JSONObject receiveMessageFromCCP() {
        try {
           JSONObject message = receiveMessage(CCPChannel);

            if (message == null) {
                return null;
            }
            
            // Sequence number handling
            int seqNum = (int)message.get("sequence_number");
            if (sequenceNumber == -1) {
                sequenceNumber = seqNum;
            } else if (seqNum == sequenceNumber + 1) {
                sequenceNumber = seqNum;
            } else {
                System.out.println("Sequence number mismatch. Expected: " + (sequenceNumber + 1) + ", Received: " + seqNum);
                // Handle sequence number error with resync mechanism
            }

            lastHeartbeatTime = System.currentTimeMillis();
            return message;
        } catch (SocketTimeoutException e) {
            // Timeout, no message received
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    private void sendMessage(DatagramChannel channel, SocketAddress ip, JSONObject message) throws Exception {
        String messageStr = message.toString();
        ByteBuffer sendBuffer = ByteBuffer.allocate(2048);
        sendBuffer.put(messageStr.getBytes());
        sendBuffer.flip();
        channel.send(sendBuffer, ip);
    }
    // MCP messages need sequence numbers, so they need a bit more fancy logic
    private void sendMessageToCCP(JSONObject message) {
        try {
            message.put("sequence_number", sequenceNumber);
            sendMessage(CCPChannel, CCPIP, message);
            // Increment sequence number
            sequenceNumber++;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void WaitForCCPConnectionMSG() {
        // Wait for AKIN response
        while (!connected) {
            JSONObject response = receiveMessageFromCCP();
            if (response != null) {
                String messageType = (String)response.get("message");
                if (messageType.equals("CCIN")) {
                    connected = true;
                    System.out.println("Connected to CCP.");
                } else {
                    System.out.println("Unexpected message: " + response.toString());
                }
            }
            System.err.println("Error in Connection");
            }
        }



    public void init() {
            WaitForCCPConnectionMSG(); 
    };
    
    public void update() {
        while(true){
            System.out.println("Running update method");


        }

    }
}
