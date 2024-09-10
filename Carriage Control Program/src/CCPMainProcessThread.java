import org.json.simple.JSONObject;
import java.nio.ByteBuffer;
import java.util.Hashtable;
import java.util.concurrent.*;

public class CCPMainProcessThread{
    ConcurrentLinkedQueue<ByteBuffer> MCPbuffers;
    ConcurrentLinkedQueue<ByteBuffer> CEPbuffers;
    CCPMainProcessThread MCPRef;
    CCPMainProcessThread CEPRef;

    //Constrcutor
    public CCPMainProcessThread(ConcurrentLinkedQueue<ByteBuffer> MCPbuffers, ConcurrentLinkedQueue<ByteBuffer> CEPbuffers, CCPMainProcessThread MCPRef, CCPMainProcessThread CEPRef){
        this.MCPbuffers=MCPbuffers;
        this.CEPbuffers=CEPbuffers;
        this.MCPRef=MCPRef;
        this.CEPRef=CEPRef;
    }

    //Functions
    public String convertToCommand(){
        //takes in data from the buffer and converts the data to
        //a string that will then be sent to the CEP thread and
        //then sent to the carriage.
        return null;
    }

    public void sendToBuffer(ByteBuffer buffer){
        String data = "test"; //replace with real data when ready
        for(byte b : data.getBytes()){
            buffer.put(b);
        }
        System.out.printf("position = %4d, limit = %4d, capacity = %4d%n",
            buffer.position(), buffer.limit(), buffer.capacity());
    }

    public void getFromBuffer(ByteBuffer buffer){
        int index = 0;
        while(buffer.hasRemaining()){
            byte data = buffer.get(index);
            char c = (char)data;
            System.out.println(c);
            index++;
        }
        System.out.println(index);
    }

    // public void sendBufferToCEPThread(ByteBuffer CEPbuffers){

    // }
    // public void sendBufferToMCPThread(ByteBuffer MCPbuffers){
    
    // }
    public void update(){

    }
}
