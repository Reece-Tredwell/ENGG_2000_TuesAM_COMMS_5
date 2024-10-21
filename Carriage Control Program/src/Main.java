public class Main {
    public static void main(String[] args) {
        // Configuration flags
        final boolean isMockingMCP = false;

        // Redirect for mock tests
        String mcpIpRedirect = (isMockingMCP) ? "" : "10.20.30.1";

        MockMCP MCP = null;
        if (isMockingMCP) {
            MCP = new MockMCP();
            MCP.init();
        }

        CCP CCP = new CCP();
        CCP.init(mcpIpRedirect, "10.20.30.110", 2000, 3010);
        
        while (true) {
            CCP.update();
            if (MCP != null) {
                MCP.update();
            }
        }
    }
}