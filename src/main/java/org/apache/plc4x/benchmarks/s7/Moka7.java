package org.apache.plc4x.benchmarks.s7;

import com.sourceforge.snap7.moka7.S7;
import com.sourceforge.snap7.moka7.S7Client;

import java.util.HashMap;
import java.util.Map;

public class Moka7 {

    public static void main(String[] args) throws Exception {
        String ipAddress = "192.168.23.30";
        int rack = 0;
        int slot = 1;

        // Create a new S7Client instance
        S7Client client = new S7Client();

        try {
            // Connect to the PLC
            int result = client.ConnectTo(ipAddress, rack, slot);
            if (result != 0) {
                throw new Exception("Connection to " + ipAddress + " failed. Error: " + S7Client.ErrorText(result));
            }

            // Define the data blocks and offsets to read
            Map<String, Object> dataMap = new HashMap<>();

            long startTime = System.currentTimeMillis();

            // Read BOOL from DB4:0.0
            boolean boolValue = readBool(client, 4, 0, 0);
            dataMap.put("BOOL", boolValue);

            // Read BYTE from DB4:1
            byte byteValue = readByte(client, 4, 1);
            dataMap.put("BYTE", byteValue);

            // Read WORD from DB4:2
            int wordValue = readWord(client, 4, 2);
            dataMap.put("WORD", wordValue);

            // Read DWORD from DB4:4
            int dwordValue = readDWord(client, 4, 4);
            dataMap.put("DWORD", dwordValue);

            // Read SINT from DB4:16
            byte sintValue = readByte(client, 4, 16);
            dataMap.put("SINT", sintValue);

            // Read USINT from DB4:17
            short usintValue = (short) (readByte(client, 4, 17) & 0xFF);
            dataMap.put("USINT", usintValue);

            // Read INT from DB4:18
            int intValue = readWord(client, 4, 18);
            dataMap.put("INT", intValue);

            // Read UINT from DB4:20
            int uintValue = readWord(client, 4, 20) & 0xFFFF;
            dataMap.put("UINT", uintValue);

            // Read DINT from DB4:22
            int dintValue = readDWord(client, 4, 22);
            dataMap.put("DINT", dintValue);

            // Read UDINT from DB4:26
            long udintValue = readDWord(client, 4, 26) & 0xFFFFFFFFL;
            dataMap.put("UDINT", udintValue);

            // Read REAL from DB4:46
            float realValue = readReal(client, 4, 46);
            dataMap.put("REAL", realValue);

            // Read LREAL from DB4:50
            double lrealValue = readLReal(client, 4, 50);
            dataMap.put("LREAL", lrealValue);

            // Read CHAR from DB4:136
            char charValue = (char) readByte(client, 4, 136);
            dataMap.put("CHAR", charValue);

            // Read WCHAR from DB4:138
            char wcharValue = (char) readWord(client, 4, 138);
            dataMap.put("WCHAR", wcharValue);

            // Read STRING(10) from DB4:140
            String stringValue = readString(client, 4, 140, 10);
            dataMap.put("STRING(10)", stringValue);

            // Read WSTRING(10) from DB4:396
            String wstringValue = readWString(client, 4, 396, 10);
            dataMap.put("WSTRING(10)", wstringValue);

            // Read TIME from DB4:58
            int timeValue = readDWord(client, 4, 58);
            dataMap.put("TIME", timeValue);

            // Read DATE from DB4:70
            int dateValue = readWord(client, 4, 70);
            dataMap.put("DATE", dateValue);

            // Read TIME_OF_DAY from DB4:72
            int todValue = readDWord(client, 4, 72);
            dataMap.put("TIME_OF_DAY", todValue);

            // Read CHAR[5] from DB4:908
            String charArrayValue = readCharArray(client, 4, 908, 5);
            dataMap.put("CHAR[5]", charArrayValue);

            // Read RAW_BYTE_ARRAY[11] from DB4:914
            byte[] rawByteArray = readBytes(client, 4, 914, 11);
            dataMap.put("RAW_BYTE_ARRAY[11]", rawByteArray);

            long endTime = System.currentTimeMillis();

            System.out.println(endTime - startTime);

        } finally {
            // Disconnect from the PLC
            client.Disconnect();
        }
    }

    // Helper methods to read different data types

    private static boolean readBool(S7Client client, int db, int byteOffset, int bitOffset) throws Exception {
        byte[] buffer = new byte[1];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 1, buffer);
        if (result != 0) {
            throw new Exception("Error reading BOOL: " + S7Client.ErrorText(result));
        }
        return S7.GetBitAt(buffer, 0, bitOffset);
    }

    private static byte readByte(S7Client client, int db, int byteOffset) throws Exception {
        byte[] buffer = new byte[1];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 1, buffer);
        if (result != 0) {
            throw new Exception("Error reading BYTE: " + S7Client.ErrorText(result));
        }
        return buffer[0];
    }

    private static int readWord(S7Client client, int db, int byteOffset) throws Exception {
        byte[] buffer = new byte[2];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 2, buffer);
        if (result != 0) {
            throw new Exception("Error reading WORD: " + S7Client.ErrorText(result));
        }
        return S7.GetWordAt(buffer, 0);
    }

    private static int readDWord(S7Client client, int db, int byteOffset) throws Exception {
        byte[] buffer = new byte[4];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 4, buffer);
        if (result != 0) {
            throw new Exception("Error reading DWORD: " + S7Client.ErrorText(result));
        }
        return S7.GetDIntAt(buffer, 0);
    }

    private static float readReal(S7Client client, int db, int byteOffset) throws Exception {
        byte[] buffer = new byte[4];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 4, buffer);
        if (result != 0) {
            throw new Exception("Error reading REAL: " + S7Client.ErrorText(result));
        }
        return S7.GetFloatAt(buffer, 0);
    }

    private static double readLReal(S7Client client, int db, int byteOffset) throws Exception {
        byte[] buffer = new byte[8];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, 8, buffer);
        if (result != 0) {
            throw new Exception("Error reading LREAL: " + S7Client.ErrorText(result));
        }
        return S7.GetFloatAt(buffer, 0);
    }

    private static String readString(S7Client client, int db, int byteOffset, int maxLength) throws Exception {
        // S7 STRING format: 1 byte for max length, 1 byte for actual length, then the string data
        byte[] buffer = new byte[2 + maxLength];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, buffer.length, buffer);
        if (result != 0) {
            throw new Exception("Error reading STRING: " + S7Client.ErrorText(result));
        }
        int actualLength = buffer[1] & 0xFF;
        if (actualLength > maxLength) {
            actualLength = maxLength;
        }
        return new String(buffer, 2, actualLength);
    }

    private static String readWString(S7Client client, int db, int byteOffset, int maxLength) throws Exception {
        // S7 WSTRING format: 2 bytes for max length, 2 bytes for actual length, then the string data (2 bytes per char)
        byte[] buffer = new byte[4 + (maxLength * 2)];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, buffer.length, buffer);
        if (result != 0) {
            throw new Exception("Error reading WSTRING: " + S7Client.ErrorText(result));
        }
        int actualLength = S7.GetWordAt(buffer, 2);
        if (actualLength > maxLength) {
            actualLength = maxLength;
        }

        char[] chars = new char[actualLength];
        for (int i = 0; i < actualLength; i++) {
            chars[i] = (char) S7.GetWordAt(buffer, 4 + (i * 2));
        }
        return new String(chars);
    }

    private static String readCharArray(S7Client client, int db, int byteOffset, int length) throws Exception {
        byte[] buffer = new byte[length];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, length, buffer);
        if (result != 0) {
            throw new Exception("Error reading CHAR array: " + S7Client.ErrorText(result));
        }
        return new String(buffer);
    }

    private static byte[] readBytes(S7Client client, int db, int byteOffset, int length) throws Exception {
        byte[] buffer = new byte[length];
        int result = client.ReadArea(S7.S7AreaDB, db, byteOffset, length, buffer);
        if (result != 0) {
            throw new Exception("Error reading byte array: " + S7Client.ErrorText(result));
        }
        return buffer;
    }
}
