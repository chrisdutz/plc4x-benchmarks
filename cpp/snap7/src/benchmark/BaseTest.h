#ifndef BASE_TEST_H
#define BASE_TEST_H

#include <string>
#include <map>
#include <thread>
#include <stdexcept>
#include <chrono>
#include "TestResults.h"

/**
 * Simple struct to represent a date.
 */
struct PlcDate {
    int year;
    int month;
    int day;

    PlcDate(int year, int month, int day) : year(year), month(month), day(day) {}

    bool operator==(const PlcDate& other) const {
        return year == other.year && month == other.month && day == other.day;
    }

    bool operator!=(const PlcDate& other) const {
        return !(*this == other);
    }
};

/**
 * Enum to identify the type of PlcValue.
 */
enum class PlcValueType {
    BOOL,
    BYTE,
    WORD,
    DWORD,
    LWORD,
    SINT,
    INT,
    DINT,
    LINT,
    USINT,
    UINT,
    UDINT,
    ULINT,
    REAL,
    LREAL,
    CHAR,
    WCHAR,
    STRING,
    WSTRING,
    TIME,
    LTIME,
    DATE,
    LDATE,
    TIME_OF_DAY,
    LTIME_OF_DAY,
    DATE_AND_TIME,
    DATE_AND_LTIME,
    LDATE_AND_TIME,
    RAW_BYTE_ARRAY
};

/*typedef unsigned char plcbyte;
typedef unsigned short plcshort;
typedef unsigned int plcint;
typedef unsigned long long plclong;*/

/**
 * Class to represent different value types that can be read from a PLC.
 */
class PlcValue {
public:
    // Default constructor
    PlcValue();

    // Constructors for different types
    explicit PlcValue(bool value);
    /*explicit PlcValue(plcbyte value);
    explicit PlcValue(plcshort value);
    explicit PlcValue(plcint value);
    explicit PlcValue(plclong value);*/
    explicit PlcValue(int8_t value);
    explicit PlcValue(int16_t value);
    explicit PlcValue(int32_t value);
    explicit PlcValue(int64_t value);
    explicit PlcValue(uint8_t value);
    explicit PlcValue(uint16_t value);
    explicit PlcValue(uint32_t value);
    explicit PlcValue(uint64_t value);
    explicit PlcValue(float value);
    explicit PlcValue(double value);
    explicit PlcValue(char value);
    explicit PlcValue(char16_t value);
    explicit PlcValue(const std::string& value);
    explicit PlcValue(const std::u16string& value);
    explicit PlcValue(std::chrono::duration<double> value);
    explicit PlcValue(const PlcDate& value);

    // Copy constructor and assignment operator
    PlcValue(const PlcValue& other);
    PlcValue& operator=(const PlcValue& other);

    // Destructor
    ~PlcValue();

    // Get the type
    PlcValueType getType() const;

    // Get the value (throws std::runtime_error if the type doesn't match)
    bool getBool() const;
    int8_t getInt8() const;
    int16_t getInt16() const;
    int32_t getInt32() const;
    int64_t getInt64() const;
    uint8_t getUint8() const;
    uint16_t getUint16() const;
    uint32_t getUint32() const;
    uint64_t getUint64() const;
    float getFloat() const;
    double getDouble() const;
    char getChar() const;
    char16_t getChar16() const;
    std::string getString() const;
    std::u16string getWstring() const;
    std::chrono::duration<double> getDuration() const;
    PlcDate getDate() const;

    // Comparison operators
    bool operator==(const PlcValue& other) const;
    bool operator!=(const PlcValue& other) const;

private:
    PlcValueType type;
    void* value;

    // Helper method to free the memory
    void freeValue();

    // Helper method to copy the value
    void copyValue(const PlcValue& other);
};

/**
 * Abstract base class for benchmark tests.
 */
class BaseTest {
public:
    //virtual ~BaseTest() = default;

    /**
     * Run the benchmark test.
     * 
     * @param numCycles Number of read cycles to perform
     * @param cycleTime Time between read cycles (in milliseconds)
     * @param tagValues Map of tag addresses to expected values
     * @return TestResults object with the benchmark results
     */
    TestResults run(int numCycles, int cycleTime, const std::map<std::string, std::string>& tagValues);

    /**
     * Get the name of the test.
     * 
     * @return Name of the test
     */
    virtual std::string getName() = 0;

    /**
     * Connect to the PLC.
     */
    virtual void connect() = 0;

    /**
     * Disconnect from the PLC.
     */
    virtual void disconnect() = 0;

    /**
     * Read values from the PLC.
     * 
     * @param tags Map of tag names to tag addresses
     * @return Map of tag names to read values
     */
    virtual std::map<std::string, PlcValue> read(const std::map<std::string, std::string>& tags) = 0;

protected:
    /**
     * Parse a value string into a PlcValue.
     * 
     * @param value Value string in the format "type;value"
     * @return Parsed value
     */
    PlcValue getValue(const std::string& value);
};

#endif // BASE_TEST_H
