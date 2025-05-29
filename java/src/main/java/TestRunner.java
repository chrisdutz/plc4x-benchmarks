import org.apache.plc4x.benchmarks.s7.Moka7;
import org.apache.plc4x.benchmarks.s7.PLC4XS7Full;
import org.apache.plc4x.benchmarks.s7.PLC4XS7Light;
import org.apache.plc4x.benchmarks.s7.S7Connector;
import org.apache.plc4x.benchmarks.s7.test.BaseTest;
import org.apache.plc4x.benchmarks.s7.test.TestResults;
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
        testRunner.runTest("PLC4X S7-Light", new PLC4XS7Light());
        testRunner.runTest("PLC4X S7", new PLC4XS7Full());
        testRunner.runTest("Moka7", new Moka7());
        testRunner.runTest("S7Connector", new S7Connector());
    }

    public void runTest(String scenarioName, BaseTest test) {
        TestResults testResults = test.run();
        int totalReadTime = 0;
        for(int i = 0 ; i < testResults.readTimes().length; i++) {
            totalReadTime += testResults.readTimes()[i];
        }
        int averageReadTime = totalReadTime / testResults.numReadCycles();
        System.out.printf("%s: %d ms connect, %d ms disconnect, %d cycles, %d ms avg read time%n", scenarioName, testResults.connectionTime(), testResults.disconnectionTime(), testResults.numReadCycles(), averageReadTime);
    }

}
