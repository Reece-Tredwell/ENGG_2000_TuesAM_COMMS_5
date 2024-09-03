import java.net.InetAddress;
import java.net.UnknownHostException;

import org.junit.*;

public class MCP_COMMS_threadTest {
    @Test
    public void test1() {
        MCP_COMMS_thread mc1 = new MCP_COMMS_thread();
        try {
            Assert.assertEquals(null, mc1.connectToMCP(InetAddress.getByName("10.20.30.110"), 3010));
        } catch (UnknownHostException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
}
