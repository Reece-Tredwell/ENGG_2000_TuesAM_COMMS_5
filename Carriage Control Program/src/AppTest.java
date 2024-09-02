import org.junit.Test;
import org.junit.*;


public class AppTest {
    @Test
    public void test() {
        App ap1 = new App();
        Assert.assertEquals("Hello, World!", ap1.main());
    }
}
