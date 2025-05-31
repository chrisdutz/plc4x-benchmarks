package org.apache.plc4x.benchmarks.s7.plc4x;

import org.apache.plc4x.benchmarks.s7.base.BaseTest;
import org.apache.plc4x.java.api.PlcConnection;
import org.apache.plc4x.java.api.PlcDriverManager;
import org.apache.plc4x.java.api.messages.PlcReadRequest;
import org.apache.plc4x.java.api.messages.PlcReadResponse;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

public class PLC4XS7Full extends BaseTest {

    public static void main(String[] args) {
        new PLC4XS7Full().run();
    }

    private PlcConnection plcConnection;

    @Override
    public void connect() throws Exception {
        String connectionString = "s7://192.168.23.30";
        plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString);
    }

    @Override
    public void disconnect() throws Exception {
        plcConnection.close();
    }

    @Override
    public Map<String, Object> read(Map<String, String> tags) throws Exception {
        PlcReadRequest.Builder builder = plcConnection.readRequestBuilder();
        tags.forEach(builder::addTagAddress);
        PlcReadRequest readRequest = builder.build();

        PlcReadResponse readResponse = readRequest.execute().get(10000, TimeUnit.MILLISECONDS);

        Map<String, Object> results = new LinkedHashMap<>();
        readRequest.getTagNames().forEach(tagName -> results.put(tagName, readResponse.getObject(tagName)));

        return results;
    }

}
