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

    private boolean connectedToMCP = false;
    private boolean connectedToCEP = false;

    private int sequenceNumber;

    long mcpLastHeartbeatTime = 0;
    long cepLastHeartbeatTime = 0;

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

            System.out.println("Setup connections");
        } catch (Exception e) {
            e.printStackTrace();
        }

        // Select sequence number
        sequenceNumber = new Random().nextInt(29001) + 1000;

        sendInitialisationMessages();
    }
    public void update() {
        JSONObject mcpResponse = receiveMessageFromMCP();
        if (mcpResponse != null) {
            String messageType = (String)mcpResponse.get("message");

            if (messageType.equals("AKIN")) {
                // Do nothing
            } else if (messageType.equals("AKST")) {
                // Do nothing
            } else if (messageType.equals("STRQ")) {

            } else if (messageType.equals("EXEC")) {
                String actionType = (String)mcpResponse.get("actions");
                // Door open is 1, door close is 0
                if (actionType == "STOPC") {
                    JSONObject closeCommand = new JSONObject();
                    closeCommand.put("cmd", "door");
                    // doesn't even fucking matter, why did I implement this field
                    closeCommand.put("timestamp", System.currentTimeMillis());
                    closeCommand.put("state", 0);
                    sendMessageToCEP(closeCommand);
                } else if (actionType == "STOPO") {
                    JSONObject openCommand = new JSONObject();
                    openCommand.put("cmd", "door");
                    // doesn't even fucking matter, why did I implement this field
                    openCommand.put("timestamp", System.currentTimeMillis());
                    openCommand.put("state", 1);
                    sendMessageToCEP(openCommand);
                } else if (actionType == "FSLOWC") {
                   JSONObject locatingCommand = new JSONObject();
                   locatingCommand.put("cmd", "locate_station");
                   sendMessageToCEP(locatingCommand); 
                } else if (actionType == "FFASTC") {
                    JSONObject fullSpeedAhead = new JSONObject();
                    fullSpeedAhead.put("cmd", "speed");
                    fullSpeedAhead.put("timestamp", System.currentTimeMillis());
                    fullSpeedAhead.put("speed", 100);
                    sendMessageToCEP(fullSpeedAhead);
                } else if (actionType == "RSLOWC") {
                    JSONObject reverseIntoStation = new JSONObject();
                    reverseIntoStation.put("cmd", "locate_station");
                   sendMessageToCEP(reverseIntoStation);
                } else if (actionType == "DISCONNECT") {
                    JSONObject shutdown = new JSONObject();
                    shutdown.put("cmd", "shutdown");
                    sendMessageToCEP(shutdown);
                } else {
                    System.out.println("Invalid actionType");
                }
            }
        }

        JSONObject cepResponse = receiveMessageFromCEP();
        if (cepResponse != null) {
            String messageType = (String)cepResponse.get("cmd");

            if (messageType.equals("message")) {
                System.out.println((String)cepResponse.get("message"));
            } else if (messageType.equals("heartbeat")) {
                cepLastHeartbeatTime = (int)cepResponse.get("timestamp");
            } else {
                System.out.print("Received unknown command: ");
                System.out.println(messageType);
            }
        }
    }
    private void sendInitialisationMessages() {
        System.out.println("Sending initialisation messages");

        // Send time initialisation message to CEP
        JSONObject timeInit = new JSONObject();
        timeInit.put("cmd", "time");
        timeInit.put("timestamp", System.currentTimeMillis());
        sendMessageToCEP(timeInit);
        System.out.println("Sent Time initialisation message");

        while (!connectedToCEP) {
            JSONObject response = receiveMessageFromCEP();
            if (response != null) {
                String messageType = (String)response.get("cmd");
                if (messageType.equals("ack")) {
                    connectedToCEP = true;
                    System.out.println("Connected to CEP");
                } else {
                    System.out.println("Unexpected CEP message: " + response.toString());
                }
            }
        }

        // Send CCIN message to MCP
        JSONObject ccinMessage = new JSONObject();
        ccinMessage.put("client_type", clientType);
        ccinMessage.put("message", "CCIN");
        ccinMessage.put("client_id", clientID);
        sendMessageToMCP(ccinMessage);
        System.out.println("Sent CCIN message");
        System.out.println("Awaiting MCP AKIN");

        // Wait for AKIN response
        while (!connectedToMCP) {
            JSONObject response = receiveMessageFromMCP();
            if (response != null) {
                String messageType = (String)response.get("message");
                if (messageType.equals("AKIN")) {
                    sequenceNumber = (int)response.get("sequence_number");
                    connectedToMCP = true;
                    System.out.println("Connected to MCP. MCP Sequence Number: " + sequenceNumber);
                } else {
                    System.out.println("Unexpected MCP message: " + response.toString());
                }
            } else {
                // Resend CCIN if no response
                sendMessageToMCP(ccinMessage);
                // Once every 200ms
                try {
                    Thread.sleep(2000);
                } catch (Exception e) {
                    // don t  car e
                }
            }
        }
    }
    private void sendMessage(DatagramChannel channel, SocketAddress ip, JSONObject message) throws Exception {
        String messageStr = message.toString();
        ByteBuffer sendBuffer = ByteBuffer.allocate(2048);
        sendBuffer.put(messageStr.getBytes());
        sendBuffer.flip();
        channel.send(sendBuffer, ip);
    }
    private JSONObject receiveMessage(DatagramChannel channel) throws Exception {
        ByteBuffer receiveBuffer = ByteBuffer.allocate(2048);
        SocketAddress sender = channel.receive(receiveBuffer);
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
    private JSONObject receiveMessageFromMCP() {
        try {
           JSONObject message = receiveMessage(mcpChannel);

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

            mcpLastHeartbeatTime = System.currentTimeMillis();
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