package org.apache.plc4x.benchmarks.s7.plc4x;

import org.apache.plc4x.java.api.PlcDriverManager;

public class PLC4XS7Light extends PLC4XS7Full {

    public PLC4XS7Light(String host, int rack, int slot) {
        super(host, rack, slot);
    }

    @Override
    public String getName() {
        return "PLC4X S7-Light";
    }

    @Override
    public void connect() throws Exception {
        String connectionString = "s7-light://" + host + "?remote-rack=" + rack + "&remote-slot=" + slot;
        plcConnection = PlcDriverManager.getDefault().getConnectionManager().getConnection(connectionString);
    }

}
