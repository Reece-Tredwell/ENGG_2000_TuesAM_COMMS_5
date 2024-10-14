import org.json.simple.JSONObject;

public class CCPMainProcess{

    public CCPMainProcess(){
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
