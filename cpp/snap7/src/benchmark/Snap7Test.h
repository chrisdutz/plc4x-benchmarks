#ifndef SNAP7_TEST_H
#define SNAP7_TEST_H

#include "BaseTest.h"
#include "../lib/snap7_libmain.h"

/**
 * Benchmark test for the snap7 library.
 */
class Snap7Test : public BaseTest {
public:
    /**
     * Constructor.
     * 
     * @param host Host name or IP address of the PLC
     * @param rack Rack number of the PLC
     * @param slot Slot number of the PLC
     */
    Snap7Test(const std::string& host, int rack, int slot);

    /**
     * Destructor.
     */
    ~Snap7Test();

    /**
     * Get the name of the test.
     * 
     * @return Name of the test
     */
    std::string getName() override;

    /**
     * Connect to the PLC.
     */
    void connect() override;

    /**
     * Disconnect from the PLC.
     */
    void disconnect() override;

    /**
     * Read values from the PLC.
     * 
     * @param tags Map of tag names to tag addresses
     * @return Map of tag names to read values
     */
    std::map<std::string, PlcValue> read(const std::map<std::string, std::string>& tags) override;

private:
    std::string host;
    int rack;
    int slot;
    S7Object client;
    bool connected;

    /**
     * Parse an S7 address string.
     * 
     * @param address Address string in the format "DB<db_number>.<byte_offset>.<bit_offset>.<data_type>"
     * @param area Output parameter for the area code
     * @param dbNumber Output parameter for the DB number
     * @param start Output parameter for the start address
     * @param wordLen Output parameter for the word length
     * @param size Output parameter for the size in bytes
     */
    void parseAddress(const std::string& address, int& area, int& dbNumber, int& start, int& wordLen, int& size, PlcValueType& type);
};

#endif // SNAP7_TEST_H