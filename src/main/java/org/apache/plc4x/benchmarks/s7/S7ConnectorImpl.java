package org.apache.plc4x.benchmarks.s7;

import com.github.s7connector.api.DaveArea;
import com.github.s7connector.api.S7Connector;
import com.github.s7connector.api.factory.S7ConnectorFactory;

import java.util.HashMap;
import java.util.Map;

public class S7ConnectorImpl {

    public static void main(String[] args) throws Exception {
        String ipAddress = "192.168.23.30";
        int rack = 0;
        int slot = 1;

        // Create a new S7Connector instance
        S7Connector connector = S7ConnectorFactory.buildTCPConnector()
                .withHost(ipAddress)
                .withRack(rack)
                .withSlot(slot)
                .build();

        try {
            // Define the data blocks and offsets to read
            Map<String, Object> dataMap = new HashMap<>();

            long startTime = System.currentTimeMillis();

            // Read BOOL from DB4:0.0
            boolean boolValue = readBool(connector, 4, 0, 0);
            dataMap.put("BOOL", boolValue);

            // Read BYTE from DB4:1
            byte byteValue = readByte(connector, 4, 1);
            dataMap.put("BYTE", byteValue);

            // Read WORD from DB4:2
            int wordValue = readWord(connector, 4, 2);
            dataMap.put("WORD", wordValue);

            // Read DWORD from DB4:4
            int dwordValue = readDWord(connector, 4, 4);
            dataMap.put("DWORD", dwordValue);

            // Read SINT from DB4:16
            byte sintValue = readByte(connector, 4, 16);
            dataMap.put("SINT", sintValue);

            // Read USINT from DB4:17
            short usintValue = (short) (readByte(connector, 4, 17) & 0xFF);
            dataMap.put("USINT", usintValue);

            // Read INT from DB4:18
            int intValue = readWord(connector, 4, 18);
            dataMap.put("INT", intValue);

            // Read UINT from DB4:20
            int uintValue = readWord(connector, 4, 20) & 0xFFFF;
            dataMap.put("UINT", uintValue);

            // Read DINT from DB4:22
            int dintValue = readDWord(connector, 4, 22);
            dataMap.put("DINT", dintValue);

            // Read UDINT from DB4:26
            long udintValue = readDWord(connector, 4, 26) & 0xFFFFFFFFL;
            dataMap.put("UDINT", udintValue);

            // Read REAL from DB4:46
            float realValue = readReal(connector, 4, 46);
            dataMap.put("REAL", realValue);

            // Read LREAL from DB4:50
            double lrealValue = readLReal(connector, 4, 50);
            dataMap.put("LREAL", lrealValue);

            // Read CHAR from DB4:136
            char charValue = (char) readByte(connector, 4, 136);
            dataMap.put("CHAR", charValue);

            // Read WCHAR from DB4:138
            char wcharValue = (char) readWord(connector, 4, 138);
            dataMap.put("WCHAR", wcharValue);

            // Read STRING(10) from DB4:140
            String stringValue = readString(connector, 4, 140, 10);
            dataMap.put("STRING(10)", stringValue);

            // Read WSTRING(10) from DB4:396
            String wstringValue = readWString(connector, 4, 396, 10);
            dataMap.put("WSTRING(10)", wstringValue);

            // Read TIME from DB4:58
            int timeValue = readDWord(connector, 4, 58);
            dataMap.put("TIME", timeValue);

            // Read DATE from DB4:70
            int dateValue = readWord(connector, 4, 70);
            dataMap.put("DATE", dateValue);

            // Read TIME_OF_DAY from DB4:72
            int todValue = readDWord(connector, 4, 72);
            dataMap.put("TIME_OF_DAY", todValue);

            // Read CHAR[5] from DB4:908
            String charArrayValue = readCharArray(connector, 4, 908, 5);
            dataMap.put("CHAR[5]", charArrayValue);

            // Read RAW_BYTE_ARRAY[11] from DB4:914
            byte[] rawByteArray = readBytes(connector, 4, 914, 11);
            dataMap.put("RAW_BYTE_ARRAY[11]", rawByteArray);

            long endTime = System.currentTimeMillis();

            System.out.println(endTime - startTime);

        } finally {
            // Close the connection
            connector.close();
        }
    }

    // Helper methods to read different data types

    private static boolean readBool(S7Connector connector, int db, int byteOffset, int bitOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 1, byteOffset);
        return ((buffer[0] >> bitOffset) & 0x01) == 0x01;
    }

    private static byte readByte(S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 1, byteOffset);
        return buffer[0];
    }

    private static int readWord(S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 2, byteOffset);
        return ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);
    }

    private static int readDWord(S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 4, byteOffset);
        return ((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
    }

    private static float readReal(S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 4, byteOffset);
        int intValue = ((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
        return Float.intBitsToFloat(intValue);
    }

    private static double readLReal(S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 8, byteOffset);
        long longValue = ((long)(buffer[0] & 0xFF) << 56) | ((long)(buffer[1] & 0xFF) << 48) | 
                         ((long)(buffer[2] & 0xFF) << 40) | ((long)(buffer[3] & 0xFF) << 32) | 
                         ((long)(buffer[4] & 0xFF) << 24) | ((long)(buffer[5] & 0xFF) << 16) | 
                         ((long)(buffer[6] & 0xFF) << 8) | (buffer[7] & 0xFF);
        return Double.longBitsToDouble(longValue);
    }

    private static String readString(S7Connector connector, int db, int byteOffset, int maxLength) throws Exception {
        // S7 STRING format: 1 byte for max length, 1 byte for actual length, then the string data
        byte[] buffer = connector.read(DaveArea.DB, db, 2 + maxLength, byteOffset);
        int actualLength = buffer[1] & 0xFF;
        if (actualLength > maxLength) {
            actualLength = maxLength;
        }
        return new String(buffer, 2, actualLength);
    }

    private static String readWString(S7Connector connector, int db, int byteOffset, int maxLength) throws Exception {
        // S7 WSTRING format: 2 bytes for max length, 2 bytes for actual length, then the string data (2 bytes per char)
        byte[] buffer = connector.read(DaveArea.DB, db, 4 + (maxLength * 2), byteOffset);
        int actualLength = ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
        if (actualLength > maxLength) {
            actualLength = maxLength;
        }

        char[] chars = new char[actualLength];
        for (int i = 0; i < actualLength; i++) {
            chars[i] = (char)(((buffer[4 + (i * 2)] & 0xFF) << 8) | (buffer[5 + (i * 2)] & 0xFF));
        }
        return new String(chars);
    }

    private static String readCharArray(S7Connector connector, int db, int byteOffset, int length) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, length, byteOffset);
        return new String(buffer);
    }

    private static byte[] readBytes(S7Connector connector, int db, int byteOffset, int length) throws Exception {
        return connector.read(DaveArea.DB, db, length, byteOffset);
    }
}