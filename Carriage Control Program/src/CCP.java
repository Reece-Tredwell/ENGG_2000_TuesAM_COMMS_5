import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.Random;

import java.nio.*;
import java.nio.channels.*;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

public class CCP {
    private final String clientType = "CCP";
    private final String clientID = "BR10";

    private DatagramChannel mcpChannel;
    private DatagramChannel cepChannel;

    private InetSocketAddress mcpIP;
    private int mcpPort;

    private InetSocketAddress cepIP;
    private int cepPort;

    private boolean connected = false;

    private int sequenceNumber;

    long lastHeartbeatTime = 0;

    public void init(String mcpIp, String cepIp, int mcpPort, int cepPort) {
        try {
            mcpChannel = DatagramChannel.open();
            mcpChannel.socket().bind(new InetSocketAddress(mcpPort));
            mcpIP = new InetSocketAddress(InetAddress.getByName(mcpIp), mcpPort);

            cepChannel = DatagramChannel.open();
            cepChannel.socket().bind(new InetSocketAddress(cepPort));
            cepIP = new InetSocketAddress(InetAddress.getByName(cepIp), cepPort);

            // We expect heartbeats every 2 seconds
            mcpChannel.socket().setSoTimeout(6000);

            mcpChannel.configureBlocking(false);
            cepChannel.configureBlocking(false);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Select sequence number
        sequenceNumber = new Random().nextInt(29001) + 1000;

        sendInitialisationMessages();
    }

    public void update() {
    }

    @SuppressWarnings("unchecked")
    private void sendInitialisationMessages() {
        // Send CCIN message to MCP
        JSONObject ccinMessage = new JSONObject();
        ccinMessage.put("client_type", clientType);
        ccinMessage.put("message", "CCIN");
        ccinMessage.put("client_id", clientID);
        ccinMessage.put("client_id", sequenceNumber);
        sendMessageToMCP(ccinMessage);

        // Wait for AKIN response
        while (!connected) {
            JSONObject response = receiveMessageFromMCP();
            if (response != null) {
                String messageType = (String) response.get("message");
                if (messageType.equals("AKIN")) {
                    sequenceNumber = (int) response.get("sequence_number");
                    connected = true;
                    System.out.println("Connected to MCP. MCP Sequence Number: " + sequenceNumber);
                } else {
                    System.out.println("Unexpected message: " + response.toString());
                }
            } else {
                // Resend CCIN if no response
                sendMessageToMCP(ccinMessage);
                // Once every 200ms
                try {
                    Thread.sleep(200);
                } catch (Exception e) {
                    // don t car e
                }
            }
        }

        // Send time initialisation message to CEP
        JSONObject timeInit = new JSONObject();
        timeInit.put("cmd", "time");
        timeInit.put("timestamp", System.currentTimeMillis());
        sendMessageToCEP(timeInit);
    }

    private void sendMessage(DatagramChannel channel, SocketAddress ip, JSONObject message) throws Exception {
        String messageStr = message.toString();
        ByteBuffer sendBuffer = ByteBuffer.allocate(2048);
        sendBuffer.put(messageStr.getBytes());
        sendBuffer.flip();
        channel.send(sendBuffer, ip);
    }

    // MCP messages need sequence numbers, so they need a bit more fancy logic
    private void sendMessageToMCP(JSONObject message) {
        try {
            message.put("sequence_number", sequenceNumber);
            sendMessage(mcpChannel, mcpIP, message);
            // Increment sequence number
            sequenceNumber++;
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Send a message to the CEP
    private void sendMessageToCEP(JSONObject message) {
        try {
            sendMessage(cepChannel, cepIP, message);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private JSONObject receiveMessage(DatagramChannel channel) throws Exception {
        ByteBuffer receiveBuffer = ByteBuffer.allocate(2048);
        SocketAddress sender = cepChannel.receive(receiveBuffer);
        JSONObject message = null;

        if (sender != null) {
            receiveBuffer.flip();
            byte[] receivedData = new byte[receiveBuffer.remaining()];
            receiveBuffer.get(receivedData);
            message = (JSONObject) new JSONParser().parse(new String(receivedData));
            receiveBuffer.clear();
        }

        return message;
    }

    private JSONObject receiveMessageFromMCP() {
        try {
            JSONObject message = receiveMessage(mcpChannel);

            if (message == null) {
                return null;
            }

            // Sequence number handling
            int seqNum = (int) message.get("sequence_number");
            if (sequenceNumber == -1) {
                sequenceNumber = seqNum;
            } else if (seqNum == sequenceNumber + 1) {
                sequenceNumber = seqNum;
            } else {
                System.out.println(
                        "Sequence number mismatch. Expected: " + (sequenceNumber + 1) + ", Received: " + seqNum);
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

    private JSONObject receiveMessageFromCEP() {
        try {
            JSONObject message = receiveMessage(cepChannel);

            return message;
        } catch (SocketTimeoutException e) {
            // Timeout, no message received
            return null;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}