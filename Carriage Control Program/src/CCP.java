import java.net.InetAddress;
import java.nio.ByteBuffer;

public class CCP{
    InetAddress mcpIP;
    int MCPPort;
    int carriageIP;
    int carriagePort;
    InetAddress CEPIP;
    MCP_COMMS_thread MCP;
    CEP_COMMS_thread CEP;
    ByteBuffer CEPBuffer = ByteBuffer.allocate(10);

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
    CCPMainProcessThread MainThread = new CCPMainProcessThread(null, null, null, null);

    MainThread.sendToBuffer(CEPBuffer);
    MainThread.getFromBuffer(CEPBuffer);
}

}