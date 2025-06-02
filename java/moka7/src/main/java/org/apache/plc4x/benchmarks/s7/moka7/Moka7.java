package org.apache.plc4x.benchmarks.s7.moka7;

import com.sourceforge.snap7.moka7.S7;
import com.sourceforge.snap7.moka7.S7Client;
import org.apache.plc4x.benchmarks.s7.base.BaseTest;
import org.apache.plc4x.java.s7.readwrite.tag.S7StringFixedLengthTag;
import org.apache.plc4x.java.s7.readwrite.tag.S7Tag;

import java.time.Duration;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;
import java.util.HashMap;
import java.util.Map;

public class Moka7 extends BaseTest {

    private final String host;
    private final int rack;
    private final int slot;
    private S7Client client;

    public Moka7(String host, int rack, int slot) {
        this.host = host;
        this.rack = rack;
        this.slot = slot;
    }

    @Override
    public String getName() {
        return "Moka7";
    }

    @Override
    public void connect() throws Exception {
        // Create a new S7Client instance
        client = new S7Client();
        int result = client.ConnectTo(host, rack, slot);
        if (result != 0) {
            throw new Exception("Connection to " + host + " failed. Error: " + S7Client.ErrorText(result));
        }
    }

    @Override
    public void disconnect() throws Exception {
        if(client != null) {
            // Disconnect from the PLC
            client.Disconnect();
        }
    }

    @Override
    public Map<String, Object> read(Map<String, String> tags) throws Exception {
        // Define the data blocks and offsets to read
        Map<String, Object> results = new HashMap<>();

        tags.forEach((name, address) -> {
            S7Tag s7Tag;
            if (S7StringFixedLengthTag.matches(address)) {
                s7Tag = S7StringFixedLengthTag.of(address);
            } else {
                s7Tag = S7Tag.of(address);
            }

            switch (s7Tag.getPlcValueType()) {
                case BOOL -> {
                    try {
                        boolean boolValue = readBool(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), s7Tag.getBitOffset());
                        results.put(name, boolValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case BYTE -> {
                    try {
                        short byteValue = readByte(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, byteValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WORD -> {
                    try {
                        int wordValue = readWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, wordValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DWORD -> {
                    try {
                        long dwordValue = ((long) readDWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset())) & 0xFFFFFFFFL;
                        results.put(name, dwordValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case SINT -> {
                    try {
                        byte sintValue = readByte(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, sintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case USINT -> {
                    try {
                        short usintValue = (short) (readByte(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFF);
                        results.put(name, usintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case INT -> {
                    try {
                        short shortValue = (short) readWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, shortValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case UINT -> {
                    try {
                        int uintValue = readWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFFFF;
                        results.put(name, uintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DINT -> {
                    try {
                        int dintValue = readDWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, dintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case UDINT -> {
                    try {
                        long udintValue = readDWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFFFFFFFFL;
                        results.put(name, udintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case REAL -> {
                    try {
                        float realValue = readReal(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, realValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case LREAL -> {
                    try {
                        double lrealValue = readLReal(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, lrealValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case CHAR -> {
                    try {
                        char charValue = (char) readByte(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, String.valueOf(charValue));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WCHAR -> {
                    try {
                        char wcharValue = (char) readWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, String.valueOf(wcharValue));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case STRING -> {
                    try {
                        S7StringFixedLengthTag fixedLengthTag = (S7StringFixedLengthTag) s7Tag;
                        String stringValue = readString(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), fixedLengthTag.getStringLength());
                        results.put(name, stringValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WSTRING -> {
                    try {
                        S7StringFixedLengthTag fixedLengthTag = (S7StringFixedLengthTag) s7Tag;
                        String wstringValue = readWString(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), fixedLengthTag.getStringLength());
                        results.put(name, wstringValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case TIME -> {
                    try {
                        int timeValue = readDWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, Duration.of(timeValue, ChronoUnit.MILLIS));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DATE -> {
                    try {
                        int daysSince1990 = readWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, LocalDate.now().withYear(1990).withDayOfMonth(1).withMonth(1).plusDays(daysSince1990));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case TIME_OF_DAY -> {
                    try {
                        int todValue = readDWord(client, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, LocalTime.ofNanoOfDay((long) todValue * 1000000));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
            }
        });

        return results;
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
        long longValue = ((buffer[0] & 0xFFL) << 56) |
                ((buffer[1] & 0xFFL) << 48) |
                ((buffer[2] & 0xFFL) << 40) |
                ((buffer[3] & 0xFFL) << 32) |
                ((buffer[4] & 0xFFL) << 24) |
                ((buffer[5] & 0xFFL) << 16) |
                ((buffer[6] & 0xFFL) <<  8) |
                ((buffer[7] & 0xFFL)) ;
        return Double.longBitsToDouble(longValue);
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
