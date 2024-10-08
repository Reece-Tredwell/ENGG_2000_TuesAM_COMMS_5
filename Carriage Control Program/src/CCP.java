import java.net.InetAddress;
import org.json.simple.JSONObject;

public class CCP{
    InetAddress mcpIP;
    int MCPPort;
    int carriageIP;
    int carriagePort;
    InetAddress CEPIP;
    MCP_COMMS MCP;
    CEP_COMMS CEP;


public CCP(){
}
public void execute(){
    /*/
    This is where all events will be called, it will contain a loop that will always be true until it is called to stop. 
    This method will be called in main
    /*/
    while(true){
        JSONObject TestObject = new JSONObject();
        TestObject.put("client_type", "ccp");
        TestObject.put("message", "SPEED");
        TestObject.put("client_id", "BRXX");
        TestObject.put("client_type", "ccp");
        TestObject.put("timestamp","2019-09-07T15:50+00Z");
        TestObject.put("action","SLOW");
        CCPMainProcess Main = new CCPMainProcess();
        CEP_COMMS CEP = new CEP_COMMS(Main.convertToCommand(TestObject), "10.20.30.110", 3010);
        CEP.writeToESP32(false);
}
}

}