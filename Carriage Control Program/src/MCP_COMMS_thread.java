import java.net.InetAddress;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import org.json.simple.JSONObject;


public class MCP_COMMS_thread{
    public JSONObject getDataFromMCP(){
    //needs to establish connection to the MCP and get the data
        JSONObject TestObject = new JSONObject();
        TestObject.put("Test", "Hello");
        System.out.println(TestObject);    
        return TestObject;
    };  

    
}

