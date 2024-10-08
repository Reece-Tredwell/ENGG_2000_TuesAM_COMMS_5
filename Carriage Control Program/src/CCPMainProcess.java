import org.json.simple.JSONObject;
import java.nio.ByteBuffer;
import java.util.Hashtable;
import java.util.concurrent.*;

public class CCPMainProcess{
    CCPMainProcess MCP;
    CCPMainProcess CEP;


    public CCPMainProcess(){
        // this.MCPbuffers=MCPbuffers;
        // this.CEPbuffers=CEPbuffers;
        // this.MCPRef=MCPRef;
        // this.CEPRef=CEPRef;
    }

    public JSONObject convertToCommand(JSONObject Data){
        //get the station
        String message = (String) Data.get("message");
        JSONObject Tester;
        System.out.println(message);
        if(message == "EXEC"){
            String action = (String) Data.get("action");
            Tester = new JSONObject();
            Tester.put("cmd",message);
            Tester.put("action",action);
        }else{
            Tester = new JSONObject();
            Tester.put("cmd",message);
        }
        System.out.println(Tester);
        return Tester;
    }

    public void update(){

    }
}
