import java.net.InetAddress;
import java.nio.ByteBuffer;

import org.json.simple.JSONObject;

public class CCP{
    InetAddress mcpIP;
    int MCPPort;
    int carriageIP;
    int carriagePort;
    InetAddress CEPIP;
    MCP_COMMS_thread MCP;
    CEP_COMMS_thread CEP;
    // ByteBuffer CEPBuffer = ByteBuffer.allocate(4);
    // ByteBuffer MCPBuffer = ByteBuffer.allocate(4);

public CCP(){//InetAddress mcpIP, int carriageIP, int carriagePort, int MCPPort, InetAddress CEPIP, MCP_COMMS_thread MCP, CEP_COMMS_thread CEP .... ADD as param in constructor when ready.
// this.mcpIP = mcpIP;
// this.MCPPort = MCPPort;
// this.carriageIP = carriageIP;
// this.carriagePort = carriagePort;
// this.CEPIP=CEPIP;
//this.MCP=MCP;
//this.CEP = CEP;
}
public void execute(){
    //This is where all events will be called,
    //it will contain a loop that will alwaysw be
    //true until it is called to stop.
    //this method will be called in main
    JSONObject TestObject = new JSONObject();
    TestObject.put("client_type", "ccp");
    TestObject.put("message", "EXEC");
    TestObject.put("client_id", "BRXX");
    TestObject.put("client_type", "ccp");
    TestObject.put("timestamp","2019-09-07T15:50+00Z");
    TestObject.put("action","SLOW");
    CCPMainProcessThread Main = new CCPMainProcessThread();
    Main.convertToCommand(TestObject);
    
// ;
//     MCP_COMMS_thread MCP = new MCP_COMMS_thread();
//     MCP.getDataFromMCP();

}

}