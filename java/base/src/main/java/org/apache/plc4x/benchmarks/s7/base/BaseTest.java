package org.apache.plc4x.benchmarks.s7.base;

import java.time.Duration;
import java.time.LocalDate;
import java.time.LocalTime;
import java.util.*;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class BaseTest {

    public TestResults run(int numCycles, int cycleTime, Map<String, String> tagValues) {
        Map<String, Object> testConfig = new LinkedHashMap<>();
        tagValues.forEach((address, valueString) -> {
            testConfig.put(address, getValue(valueString));
        });

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
            int connectionTime;
            int disconnectionTime;
            int[] readTimes = new int[numCycles];
            try {
                long startTime = System.currentTimeMillis();
                connect();
                long endTime = System.currentTimeMillis();
                connectionTime = (int) (endTime - startTime);

                AtomicInteger cycleNumber = new AtomicInteger(0);
                CompletableFuture<Void> finished = new CompletableFuture<>();
                Timer timer = new Timer();
                timer.schedule(new TimerTask() {
                    @Override
                    public void run() {
                        int curCycleNumber = cycleNumber.incrementAndGet();
                        try {
                            long startTime = System.currentTimeMillis();
                            Map<String, Object> results = read(tags);
                            long endTime = System.currentTimeMillis();
                            int readTime = (int) (endTime - startTime);

                            // Check the results
                            results.forEach((k, v) -> {
                                if (v instanceof Throwable) {
                                    throw new RuntimeException("Error during read", (Throwable) v);
                                }
                                if (!expectedResults.containsKey(k)) {
                                    throw new RuntimeException("Unexpected result: " + k + " = " + v + " for tag address " + tags.get(k));
                                }
                                if (!expectedResults.get(k).equals(v)) {
                                    throw new RuntimeException("Unexpected result: " + k + " = " + v + " (expected: " + expectedResults.get(k) + ")" + " for tag address " + tags.get(k));
                                }
                            });

                            readTimes[curCycleNumber] = readTime;

                            if (curCycleNumber == numCycles - 1) {
                                finished.complete(null);
                            }
                        } catch (Exception e) {
                            finished.completeExceptionally(e);
                        }
                    }
                }, 0, cycleTime);
                finished.get();
                timer.cancel();
            } catch (Exception e) {
                throw new RuntimeException("Error during test", e);
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

    protected Object getValue(String value) {
        String typeString = value.split(";")[0];
        String valueString = value.split(";")[1];
        return switch (typeString) {
            case "boolean" -> Boolean.parseBoolean(valueString);
            case "byte" -> Byte.parseByte(valueString);
            case "short" -> Short.parseShort(valueString);
            case "int" -> Integer.parseInt(valueString);
            case "long" -> Long.parseLong(valueString);
            case "float" -> Float.parseFloat(valueString);
            case "double" -> Double.parseDouble(valueString);
            case "char" -> valueString.substring(0, 1);
            case "string" -> valueString;
            case "time" -> Duration.parse(valueString);
            case "date" -> LocalDate.parse(valueString);
            case "time_of_day" -> LocalTime.parse(valueString);
            default -> throw new RuntimeException("Unknown type: " + typeString);
        };
    }

    abstract public String getName();

    abstract public void connect() throws Exception;

    abstract public void disconnect() throws Exception;

    abstract public Map<String, Object> read(Map<String, String> tags) throws Exception;

}
