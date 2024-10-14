import org.json.simple.JSONObject;
public class CCPMainProcess{
    CCPMainProcess MCP;
    CCPMainProcess CEP;
    public CCPMainProcess(){
        this.MCP=MCP;
        this.CEP=CEP;
    }
    public JSONObject convertToCommand(JSONObject Data){
        //get the station
        String message = (String) Data.get("message");
        JSONObject command;
        System.out.println(message);
        if(message == "EXEC"){
            String action = (String) Data.get("action");
            command = new JSONObject();
            command.put("cmd",message);
            command.put("action",action);
        }else{
            command = new JSONObject();
            command.put("cmd",message);
        }
        System.out.println(command);
        return command;
    }
}
