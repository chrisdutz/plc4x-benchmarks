package org.apache.plc4x.benchmarks.s7;

import org.apache.plc4x.benchmarks.s7.base.BaseTest;
import org.apache.plc4x.benchmarks.s7.base.TestResults;
import org.apache.plc4x.benchmarks.s7.moka7.Moka7;
import org.apache.plc4x.benchmarks.s7.plc4x.PLC4XS7Full;
import org.apache.plc4x.benchmarks.s7.plc4x.PLC4XS7Light;
import org.apache.plc4x.benchmarks.s7.s7connect.S7Connector;
import org.apache.plc4x.java.api.PlcConnection;
import org.apache.plc4x.java.api.PlcDriverManager;
import org.junit.jupiter.api.Test;

import java.util.HashMap;
import java.util.Map;

public class S7BenchmarkTest {

    @Test
    public void run() {
        String host = System.getProperty("host");
        int remoteRack = Integer.parseInt(System.getProperty("remoteRack", "-1"));
        int remoteSlot = Integer.parseInt(System.getProperty("remoteSlot", "-1"));
        int numCycles = Integer.parseInt(System.getProperty("numCycles", "-1"));
        int cycleTime = Integer.parseInt(System.getProperty("cycleTime", "-1"));
        Map<String, String> tagValueStrings = new HashMap<>();
        String tagStringList = System.getProperty("tags", "");
        String[] tagStrings = tagStringList.split("\n");
        for (String tagString : tagStrings) {
            String[] split = tagString.trim().split("\\|");
            if(split.length != 2) {
                continue;
            }
            String address = split[0].trim();
            String value = split[1].trim();
            tagValueStrings.put(address, value);
        }

        // Create a fake connection to wake the device up.
        // It seems the first connection takes a while.
        String connectionString = "s7://" + host;
        try (PlcConnection plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString)) {
            if(!plcConnection.getMetadata().isReadSupported()) {
                throw new Exception("Read not supported");
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

        System.out.printf("Scenario: %d tags, %d cycles, %dms intervals%n%n", tagValueStrings.size(), numCycles, cycleTime);

        // Do the actual tests.
        runTest(new PLC4XS7Light(host, remoteRack, remoteSlot), numCycles, cycleTime, tagValueStrings);
        runTest(new PLC4XS7Full(host, remoteRack, remoteSlot), numCycles, cycleTime, tagValueStrings);
        runTest(new Moka7(host, remoteRack, remoteSlot), numCycles, cycleTime, tagValueStrings);
        runTest(new S7Connector(host, remoteRack, remoteSlot), numCycles, cycleTime, tagValueStrings);
    }

    public static void runTest(BaseTest test, int numCycles, int cycleTime, Map<String, String> tagValueStrings) {
        System.out.printf("Running: '%s'%n", test.getName());
        TestResults testResults = test.run(numCycles, cycleTime, tagValueStrings);
        int totalReadTime = 0;
        for(int i = 0 ; i < testResults.readTimes().length; i++) {
            totalReadTime += testResults.readTimes()[i];
        }
        int averageReadTime = totalReadTime / testResults.numReadCycles();
        System.out.printf("  --> %d ms connect, %d ms disconnect, %d ms avg read time%n", testResults.connectionTime(), testResults.disconnectionTime(), averageReadTime);
    }

}
