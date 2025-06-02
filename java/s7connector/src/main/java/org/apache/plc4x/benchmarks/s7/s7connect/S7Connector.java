package org.apache.plc4x.benchmarks.s7.s7connect;

import com.github.s7connector.api.DaveArea;
import com.github.s7connector.api.factory.S7ConnectorFactory;
import org.apache.plc4x.benchmarks.s7.base.BaseTest;
import org.apache.plc4x.java.s7.readwrite.tag.S7StringFixedLengthTag;
import org.apache.plc4x.java.s7.readwrite.tag.S7Tag;

import java.time.Duration;
import java.time.LocalDate;
import java.time.LocalTime;
import java.time.temporal.ChronoUnit;
import java.util.HashMap;
import java.util.Map;

public class S7Connector extends BaseTest {

    private final String host;
    private final int rack;
    private final int slot;
    private com.github.s7connector.api.S7Connector connector;

    public S7Connector(String host, int rack, int slot) {
        this.host = host;
        this.rack = rack;
        this.slot = slot;
    }

    @Override
    public String getName() {
        return "S7Connector";
    }

    @Override
    public void connect() throws Exception {
        connector = S7ConnectorFactory.buildTCPConnector()
                .withHost(host)
                .withRack(rack)
                .withSlot(slot)
                .build();
    }

    @Override
    public void disconnect() throws Exception {
        if(connector != null) {
            // Close the connection
            connector.close();
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
                        boolean boolValue = readBool(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), s7Tag.getBitOffset());
                        results.put(name, boolValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case BYTE -> {
                    try {
                        short byteValue = readByte(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, byteValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WORD -> {
                    try {
                        int wordValue = readWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, wordValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DWORD -> {
                    try {
                        long dwordValue =  ((long) readDWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset())) & 0xFFFFFFFFL;
                        results.put(name, dwordValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case SINT -> {
                    try {
                        byte sintValue = readByte(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, sintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case USINT -> {
                    try {
                        short usintValue = (short) (readByte(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFF);
                        results.put(name, usintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case INT -> {
                    try {
                        short shortValue = (short) readWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, shortValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case UINT -> {
                    try {
                        int uintValue = readWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFFFF;
                        results.put(name, uintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DINT -> {
                    try {
                        int dintValue = readDWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, dintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case UDINT -> {
                    try {
                        long udintValue = readDWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset()) & 0xFFFFFFFFL;
                        results.put(name, udintValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case REAL -> {
                    try {
                        float realValue = readReal(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, realValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case LREAL -> {
                    try {
                        double lrealValue = readLReal(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, lrealValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case CHAR -> {
                    try {
                        char charValue = (char) readByte(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, String.valueOf(charValue));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WCHAR -> {
                    try {
                        char wcharValue = (char) readWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, String.valueOf(wcharValue));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case STRING -> {
                    try {
                        S7StringFixedLengthTag fixedLengthTag = (S7StringFixedLengthTag) s7Tag;
                        String stringValue = readString(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), fixedLengthTag.getStringLength());
                        results.put(name, stringValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case WSTRING -> {
                    try {
                        S7StringFixedLengthTag fixedLengthTag = (S7StringFixedLengthTag) s7Tag;
                        String wstringValue = readWString(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset(), fixedLengthTag.getStringLength());
                        results.put(name, wstringValue);
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case TIME -> {
                    try {
                        int timeValue = readDWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, Duration.of(timeValue, ChronoUnit.MILLIS));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case DATE -> {
                    try {
                        int daysSince1990 = readWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
                        results.put(name, LocalDate.now().withYear(1990).withDayOfMonth(1).withMonth(1).plusDays(daysSince1990));
                    } catch (Exception e) {
                        results.put(name, e);
                    }
                }
                case TIME_OF_DAY -> {
                    try {
                        int todValue = readDWord(connector, s7Tag.getBlockNumber(), s7Tag.getByteOffset());
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

    private static boolean readBool(com.github.s7connector.api.S7Connector connector, int db, int byteOffset, int bitOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 1, byteOffset);
        return ((buffer[0] >> bitOffset) & 0x01) == 0x01;
    }

    private static byte readByte(com.github.s7connector.api.S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 1, byteOffset);
        return buffer[0];
    }

    private static int readWord(com.github.s7connector.api.S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 2, byteOffset);
        return ((buffer[0] & 0xFF) << 8) | (buffer[1] & 0xFF);
    }

    private static int readDWord(com.github.s7connector.api.S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 4, byteOffset);
        return ((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
    }

    private static float readReal(com.github.s7connector.api.S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 4, byteOffset);
        int intValue = ((buffer[0] & 0xFF) << 24) | ((buffer[1] & 0xFF) << 16) | ((buffer[2] & 0xFF) << 8) | (buffer[3] & 0xFF);
        return Float.intBitsToFloat(intValue);
    }

    private static double readLReal(com.github.s7connector.api.S7Connector connector, int db, int byteOffset) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, 8, byteOffset);
        long longValue = ((long)(buffer[0] & 0xFF) << 56) | ((long)(buffer[1] & 0xFF) << 48) | 
                         ((long)(buffer[2] & 0xFF) << 40) | ((long)(buffer[3] & 0xFF) << 32) | 
                         ((long)(buffer[4] & 0xFF) << 24) | ((long)(buffer[5] & 0xFF) << 16) | 
                         ((long)(buffer[6] & 0xFF) << 8) | (buffer[7] & 0xFF);
        return Double.longBitsToDouble(longValue);
    }

    private static String readString(com.github.s7connector.api.S7Connector connector, int db, int byteOffset, int maxLength) throws Exception {
        // S7 STRING format: 1 byte for max length, 1 byte for actual length, then the string data
        byte[] buffer = connector.read(DaveArea.DB, db, 2 + maxLength, byteOffset);
        int actualLength = buffer[1] & 0xFF;
        if (actualLength > maxLength) {
            actualLength = maxLength;
        }
        return new String(buffer, 2, actualLength);
    }

    private static String readWString(com.github.s7connector.api.S7Connector connector, int db, int byteOffset, int maxLength) throws Exception {
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

    private static String readCharArray(com.github.s7connector.api.S7Connector connector, int db, int byteOffset, int length) throws Exception {
        byte[] buffer = connector.read(DaveArea.DB, db, length, byteOffset);
        return new String(buffer);
    }

    private static byte[] readBytes(com.github.s7connector.api.S7Connector connector, int db, int byteOffset, int length) throws Exception {
        return connector.read(DaveArea.DB, db, length, byteOffset);
    }
}