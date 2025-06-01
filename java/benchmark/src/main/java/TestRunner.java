import org.apache.plc4x.benchmarks.s7.base.BaseTest;
import org.apache.plc4x.benchmarks.s7.base.TestResults;
import org.apache.plc4x.benchmarks.s7.moka7.Moka7;
import org.apache.plc4x.benchmarks.s7.plc4x.PLC4XS7Full;
import org.apache.plc4x.benchmarks.s7.plc4x.PLC4XS7Light;
import org.apache.plc4x.benchmarks.s7.s7connect.S7Connector;
import org.apache.plc4x.java.api.PlcConnection;
import org.apache.plc4x.java.api.PlcDriverManager;

public class TestRunner {

    public static void main(String[] args) {
        // Create a fake connection to wake the device up.
        String connectionString = "s7://192.168.23.30";
        try (PlcConnection plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString)) {
            if(!plcConnection.getMetadata().isReadSupported()) {
                throw new Exception("Read not supported");
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

        // Do the actual tests.
        TestRunner testRunner = new TestRunner();
        testRunner.runTest("PLC4X S7-Light", "192.168.23.30", 0, 0, new PLC4XS7Light());
        testRunner.runTest("PLC4X S7", "192.168.23.30", 0, 0, new PLC4XS7Full());
        testRunner.runTest("Moka7", "192.168.23.30", 0, 0, new Moka7());
        testRunner.runTest("S7Connector", "192.168.23.30", 0, 0, new S7Connector());
    }

    public void runTest(String scenarioName, String host, int rack, int slot, BaseTest test) {
        TestResults testResults = test.run(host, rack, slot);
        int totalReadTime = 0;
        for(int i = 0 ; i < testResults.readTimes().length; i++) {
            totalReadTime += testResults.readTimes()[i];
        }
        int averageReadTime = totalReadTime / testResults.numReadCycles();
        System.out.printf("%s: %d ms connect, %d ms disconnect, %d cycles, %d ms avg read time%n", scenarioName, testResults.connectionTime(), testResults.disconnectionTime(), testResults.numReadCycles(), averageReadTime);
    }

}
