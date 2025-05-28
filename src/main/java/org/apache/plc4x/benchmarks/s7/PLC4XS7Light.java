package org.apache.plc4x.benchmarks.s7;

import org.apache.plc4x.java.api.PlcConnection;
import org.apache.plc4x.java.api.PlcDriverManager;
import org.apache.plc4x.java.api.messages.PlcReadRequest;

public class PLC4XS7Light {

    public static void main(String[] args) throws Exception {
        String connectionString = "s7://192.168.23.30";

        try (PlcConnection plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString)) {
            PlcReadRequest.Builder builder = plcConnection.readRequestBuilder();
            builder.addTagAddress("BOOL", "%DB4:0.0:BOOL");
            builder.addTagAddress("BYTE", "%DB4:1:BYTE");
            builder.addTagAddress("WORD", "%DB4:2:WORD");
            builder.addTagAddress("DWORD", "%DB4:4:DWORD");
            builder.addTagAddress("SINT", "%DB4:16:SINT");
            builder.addTagAddress("USINT", "%DB4:17:USINT");
            builder.addTagAddress("INT", "%DB4:18:INT");
            builder.addTagAddress("UINT", "%DB4:20:UINT");
            builder.addTagAddress("DINT", "%DB4:22:DINT");
            builder.addTagAddress("UDINT", "%DB4:26:UDINT");
            builder.addTagAddress("REAL", "%DB4:46:REAL");
            builder.addTagAddress("LREAL", "%DB4:50:LREAL");
            builder.addTagAddress("CHAR", "%DB4:136:CHAR");
            builder.addTagAddress("WCHAR", "%DB4:138:WCHAR");
            builder.addTagAddress("STRING(10)", "%DB4:140:STRING(10)");
            builder.addTagAddress("WSTRING(10)", "%DB4:396:WSTRING(10)");
            builder.addTagAddress("TIME", "%DB4:58:TIME");
            builder.addTagAddress("DATE", "%DB4:70:DATE");
            builder.addTagAddress("TIME_OF_DAY", "%DB4:72:TIME_OF_DAY");
            builder.addTagAddress("CHAR[5]", "%DB4:908:CHAR[5]");
            builder.addTagAddress("RAW_BYTE_ARRAY[11]", "%DB4:914:RAW_BYTE_ARRAY[11]");
            PlcReadRequest readRequest = builder.build();

            long startTime = System.currentTimeMillis();
            readRequest.execute();
            long endTime = System.currentTimeMillis();

            System.out.println(endTime - startTime);
        }
    }

}
