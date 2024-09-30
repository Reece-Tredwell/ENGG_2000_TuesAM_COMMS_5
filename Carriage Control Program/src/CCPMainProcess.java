import org.json.simple.JSONObject;
import java.nio.ByteBuffer;
import java.util.Hashtable;
import java.util.concurrent.*;

public class CCPMainProcess{

    // ConcurrentLinkedQueue<ByteBuffer> MCPbuffers;
    // ConcurrentLinkedQueue<ByteBuffer> CEPbuffers;
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
            Tester.put("Command",message);
            Tester.put("action",action);
        }else{
            Tester = new JSONObject();
            Tester.put("Command",message);
        }
        System.out.println(Tester);
        return Tester;
    }

    // public void getFromBuffer(ByteBuffer buffer){
    //     int index = 0;
    //     while(buffer.hasRemaining()){
    //         byte data = buffer.get(index);
    //         char c = (char)data;
    //         System.out.println(c);
    //         index++;
    //     }
    //     System.out.println(index);
    // }

    // public void sendBufferToCEPThread(ByteBuffer CEPbuffers){

    // }
    // public void sendBufferToMCPThread(ByteBuffer MCPbuffers){
    
    // }
    public void update(){

    }
}
