import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketTimeoutException;
import java.util.Random;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;

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

    private boolean connectedToCEP = false;
    private boolean connectedToMCP = false;
    
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
            cepChannel.socket().setSoTimeout(6000);

            mcpChannel.configureBlocking(false);
            cepChannel.configureBlocking(false);

            System.out.println("Setup connections");
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }

        // Select sequence number
        sequenceNumber = new Random().nextInt(29001) + 1000;

        sendInitialisationMessages();

        // Once initialised, open UI
        System.out.println("Opening UI");
        JFrame frame = new JFrame("Packet Sender UI");
        frame.setSize(400, 300);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(null);

        // Button for STOPC
        JButton stopcButton = new JButton("STOPC");
        stopcButton.setBounds(50, 20, 100, 30);
        stopcButton.addActionListener(e -> {
           onDoor(false);
        });
        frame.add(stopcButton);

        // Button for STOPO
        JButton stopoButton = new JButton("STOPO");
        stopoButton.setBounds(200, 20, 100, 30);
        stopoButton.addActionListener(e -> {
            onDoor(true);
        });
        frame.add(stopoButton);

        // Button for FSLOWC
        JButton fslowcButton = new JButton("FSLOWC");
        fslowcButton.setBounds(50, 70, 100, 30);
        fslowcButton.addActionListener(e -> {
            onLocateStation();
        });
        frame.add(fslowcButton);

        // Button and field for FFASTC
        JLabel speedLabel = new JLabel("Speed:");
        speedLabel.setBounds(200, 70, 50, 30);
        frame.add(speedLabel);

        JTextField speedField = new JTextField("100");  // Default speed
        speedField.setBounds(250, 70, 50, 30);
        frame.add(speedField);

        JButton ffastcButton = new JButton("FFASTC");
        ffastcButton.setBounds(50, 120, 100, 30);
        ffastcButton.addActionListener(e -> {
            int speed;
            try {
                speed = Integer.parseInt(speedField.getText());
            } catch (NumberFormatException ex) {
                JOptionPane.showMessageDialog(frame, "Invalid speed value");
                return;
            }
            onSpeedCommand(speed);
        });
        frame.add(ffastcButton);

        // Button for RSLOWC
        JButton rslowcButton = new JButton("RSLOWC");
        rslowcButton.setBounds(200, 120, 100, 30);
        rslowcButton.addActionListener(e -> {
            onReverseLocateStation();
        });
        frame.add(rslowcButton);

        // Button for DISCONNECT
        JButton disconnectButton = new JButton("DISCONNECT");
        disconnectButton.setBounds(125, 170, 100, 30);
        disconnectButton.addActionListener(e -> {
            onDisconnect();
        });
        frame.add(disconnectButton);

        frame.setVisible(true);
    }
    public void onStop() {
        JSONObject stopCommand = new JSONObject();
        stopCommand.put("cmd", "stop");
        // doesn't even fucking matter, why did I implement this field
        stopCommand.put("timestamp", System.currentTimeMillis());
        sendMessageToCEP(stopCommand);
    }
    public void onDoor(boolean state) {
        JSONObject closeCommand = new JSONObject();
        closeCommand.put("cmd", "door");
        // doesn't even fucking matter, why did I implement this field
        closeCommand.put("timestamp", System.currentTimeMillis());
        closeCommand.put("state", state);
        sendMessageToCEP(closeCommand);
    }
    public void onLocateStation() {
        JSONObject locatingCommand = new JSONObject();
        locatingCommand.put("cmd", "locate_station");
        sendMessageToCEP(locatingCommand); 
    }
    public void onSpeedCommand(int speed) {
        JSONObject fullSpeedAhead = new JSONObject();
        fullSpeedAhead.put("cmd", "speed");
        fullSpeedAhead.put("timestamp", System.currentTimeMillis());
        fullSpeedAhead.put("speed", speed);
        sendMessageToCEP(fullSpeedAhead);
    }
    public void onReverseLocateStation() {
        JSONObject reverseIntoStation = new JSONObject();
        reverseIntoStation.put("cmd", "locate_station");
       sendMessageToCEP(reverseIntoStation);
    }
    public void onDisconnect() {
        JSONObject shutdown = new JSONObject();
        shutdown.put("cmd", "shutdown");
        sendMessageToCEP(shutdown);
    }
    public JSONObject generateStatus(String statusType) {
        JSONObject status = new JSONObject();
        status.put("client_type", "CCP");
        status.put("message", "STAT");
        status.put("client_id", "BR10");
        status.put("status", statusType);

        return status;
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
                JSONObject status = generateStatus("FFASTC");
                sendMessageToMCP(status);
            } else if (messageType.equals("EXEC")) {
                String actionType = (String)mcpResponse.get("action");
                // Door open is 1, door close is 0
                if (actionType.equals("STOPC")) {
                    onDoor(false);
                } else if (actionType.equals("STOPO")) {
                    onStop();
                    onDoor(true);
                } else if (actionType.equals("FSLOWC")) {
                   onLocateStation();
                } else if (actionType.equals("FFASTC")) {
                   onSpeedCommand(100);
                } else if (actionType.equals("RSLOWC")) {
                    onReverseLocateStation();
                } else if (actionType.equals("DISCONNECT")) {
                    onDisconnect();
                    sendMessageToMCP(generateStatus("OFLN"));
                } else {
                    System.out.print("Invalid actionType: ");
                    System.out.println(actionType);
                }
            }

            // Send acknowledgement
            JSONObject ack = new JSONObject();
            ack.put("client_type", "CCP");
            ack.put("message", "AKEX");
            ack.put("client_id", "BR10");
            sendMessageToMCP(ack);
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

        // Assume we lost connection to CEP
        if (cepLastHeartbeatTime != 0 && System.currentTimeMillis() - cepLastHeartbeatTime > 6000) {
            System.out.println("Lost connection to the CEP");
            connectedToCEP = false;
            sendMessageToMCP(generateStatus("ERR"));
        }

        if (mcpLastHeartbeatTime != 0 && System.currentTimeMillis() - mcpLastHeartbeatTime > 6000) {
            System.out.println("Lost connection to the MCP");
            connectedToMCP = false;
            onStop();
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
        ccinMessage.put("sequence_number", sequenceNumber);
        sendMessageToMCP(ccinMessage);
        System.out.println("Sent CCIN message");
        System.out.println("Awaiting MCP AKIN");

        // Wait for AKIN response
        while (!connectedToMCP) {
            JSONObject response = receiveMessageFromMCP();
            if (response != null) {
                String messageType = (String) response.get("message");
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
                    Thread.sleep(20000);
                } catch (Exception e) {
                    // dont care
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