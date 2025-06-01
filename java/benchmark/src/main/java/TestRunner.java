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
        // It seesm the first connection takes a while.
        // We're now also using this connection throughout the test for collecting telemetry from the PLC.
        String connectionString = "s7://192.168.23.30";
        try (PlcConnection plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString)) {
            if(!plcConnection.getMetadata().isReadSupported()) {
                throw new Exception("Read not supported");
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

        int numCycles = 50;
        int cycleTime = 300;

        // Do the actual tests.
        TestRunner testRunner = new TestRunner();
        testRunner.runTest("PLC4X S7-Light", "192.168.23.30", 0, 0, new PLC4XS7Light(), numCycles, cycleTime);
        testRunner.runTest("PLC4X S7", "192.168.23.30", 0, 0, new PLC4XS7Full(), numCycles, cycleTime);
        testRunner.runTest("Moka7", "192.168.23.30", 0, 0, new Moka7(), numCycles, cycleTime);
        testRunner.runTest("S7Connector", "192.168.23.30", 0, 0, new S7Connector(), numCycles, cycleTime);
    }

    public void runTest(String scenarioName, String host, int rack, int slot, BaseTest test, int numCycles, int cycleTime) {
        TestResults testResults = test.run(host, rack, slot, numCycles, cycleTime);
        int totalReadTime = 0;
        for(int i = 0 ; i < testResults.readTimes().length; i++) {
            totalReadTime += testResults.readTimes()[i];
        }
        int averageReadTime = totalReadTime / testResults.numReadCycles();
        System.out.printf("%s: %d ms connect, %d ms disconnect, %d cycles, %d ms avg read time%n", scenarioName, testResults.connectionTime(), testResults.disconnectionTime(), testResults.numReadCycles(), averageReadTime);
    }

}
