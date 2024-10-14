public class Main {
    public static void main(String[] args) {
        CCP CCP = new CCP();
        CCP.init("10.20.30.1", "10.20.30.110", 2000, 3010);
        
        while (true) {
            CCP.update();
        }
    }
}
