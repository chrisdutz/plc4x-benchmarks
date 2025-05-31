package org.apache.plc4x.benchmarks.s7.base;

import java.time.Duration;
import java.time.LocalDate;
import java.time.LocalTime;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class BaseTest {

    public TestResults run() {
        Map<String, Object> testConfig = new LinkedHashMap<>();
        testConfig.put("%DB4:0.0:BOOL", true);
        testConfig.put("%DB4:1:BYTE", (short) 42);
        testConfig.put("%DB4:2:WORD", (int) 42424);
        testConfig.put("%DB4:4:DWORD", (long) 4242442424L);
        testConfig.put("%DB4:16:SINT", (byte) -42);
        testConfig.put("%DB4:17:USINT", (short) 42);
        testConfig.put("%DB4:18:INT", (short) -2424);
        testConfig.put("%DB4:20:UINT", (int) 42424);
        testConfig.put("%DB4:22:DINT", (int) -242442424);
        testConfig.put("%DB4:26:UDINT", (long) 4242442424L);
        testConfig.put("%DB4:46:REAL", (float) 3.141593F);
        testConfig.put("%DB4:50:LREAL", (double) 2.71828182846D);
        testConfig.put("%DB4:136:CHAR", "H");
        testConfig.put("%DB4:138:WCHAR", "w");
        testConfig.put("%DB4:140:STRING(10)", "hurz");
        testConfig.put("%DB4:396:WSTRING(10)", "wolf");
        testConfig.put("%DB4:58:TIME", Duration.parse("PT1.234S"));
        testConfig.put("%DB4:70:DATE", LocalDate.parse("1998-03-28"));
        testConfig.put("%DB4:72:TIME_OF_DAY", LocalTime.parse("15:36:30.123"));
//        testConfig.put("%DB4:908:CHAR[5]", Arrays.asList(new PlcCHAR("w"), new PlcCHAR("i"), new PlcCHAR("e"), new PlcCHAR("s"), new PlcCHAR("e")));
//        testConfig.put("%DB4:914:RAW_BYTE_ARRAY[11]", new byte[] {(byte) 1, (byte) 2, (byte) 3, (byte) 4, (byte) 5, (byte) 6, (byte) 7, (byte) 8, (byte) 9, (byte) 10, (byte) 11});

        // Prepare the input and expected output maps
        Map<String, String> tags = new LinkedHashMap<>(testConfig.size());
        Map<String, Object> expectedResults = new HashMap<>(testConfig.size());
        AtomicInteger tagNumber = new AtomicInteger(0);
        testConfig.forEach((k, v) -> {
            int curTagNumber = tagNumber.incrementAndGet();
            tags.put("tag-" + curTagNumber, k);
            expectedResults.put("tag-" + curTagNumber, v);
        });

        // Execute the read operation
        try {
            int connectionTime = 0;
            int disconnectionTime = 0;
            int numCycles = 300;
            int[] readTimes = new int[numCycles];
            try {
                long startTime = System.currentTimeMillis();
                connect();
                long endTime = System.currentTimeMillis();
                connectionTime = (int) (endTime - startTime);

                for (int i = 0; i < 300; i++) {
                    startTime = System.currentTimeMillis();
                    Map<String, Object> results = read(tags);
                    endTime = System.currentTimeMillis();
                    int readTime = (int) (endTime - startTime);

                    // Check the results
                    results.forEach((k, v) -> {
                        if(v instanceof Throwable) {
                            throw new RuntimeException("Error during read", (Throwable) v);
                        }
                        if (!expectedResults.containsKey(k)) {
                            throw new RuntimeException("Unexpected result: " + k + " = " + v + " for tag address " + tags.get(k));
                        }
                        if (!expectedResults.get(k).equals(v)) {
                            throw new RuntimeException("Unexpected result: " + k + " = " + v + " (expected: " + expectedResults.get(k) + ")" + " for tag address " + tags.get(k));
                        }
                    });

                    readTimes[i] = readTime;
                }
            } finally {
                long startTime = System.currentTimeMillis();
                disconnect();
                long endTime = System.currentTimeMillis();
                disconnectionTime = (int) (endTime - startTime);
            }

            return new TestResults(connectionTime, disconnectionTime, numCycles, readTimes);
        } catch (Exception e) {
            throw new RuntimeException("Error during test", e);
        }
    }

    abstract public void connect() throws Exception;

    abstract public void disconnect() throws Exception;

    abstract public Map<String, Object> read(Map<String, String> tags) throws Exception;

}
