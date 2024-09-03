import java.net.InetAddress;

public class CCP{
    InetAddress mcpIP;
    int MCPPort;
    int carriageIP;
    int carriagePort;
    InetAddress CEPIP;
    MCP_COMMS_thread MCP;
    CEP_COMMS_thread CEP;

public CCP(InetAddress mcpIP, int carriageIP, int carriagePort, int MCPPort, InetAddress CEPIP, MCP_COMMS_thread MCP, CEP_COMMS_thread CEP){
this.mcpIP = mcpIP;
this.MCPPort = MCPPort;
this.carriageIP = carriageIP;
this.carriagePort = carriagePort;
this.CEPIP=CEPIP;
this.MCP=MCP;
this.CEP = CEP;
}
public void execute(){
    //This is where all events will be called,
    //it will contain a loop that will alwaysw be
    //true until it is called to stop.
    //this method will be called in main
}

}