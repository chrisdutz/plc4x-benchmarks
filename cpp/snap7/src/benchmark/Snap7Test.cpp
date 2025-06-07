#include "Snap7Test.h"
#include <regex>
#include <cstring>

Snap7Test::Snap7Test(const std::string& host, int rack, int slot)
    : host(host), rack(rack), slot(slot), client(0), connected(false) {
}

Snap7Test::~Snap7Test() {
    if (connected) {
        disconnect();
    }
    if (client) {
        Cli_Destroy(client);
    }
}

std::string Snap7Test::getName() {
    return "Snap7";
}

void Snap7Test::connect() {
    if (connected) {
        return;
    }

    client = Cli_Create();
    if (!client) {
        throw std::runtime_error("Failed to create Snap7 client");
    }

    int result = Cli_ConnectTo(client, host.c_str(), rack, slot);
    if (result != 0) {
        char errorText[1024];
        Cli_ErrorText(result, errorText, sizeof(errorText));
        throw std::runtime_error("Failed to connect to PLC: " + std::string(errorText));
    }

    connected = true;
}

void Snap7Test::disconnect() {
    if (!connected) {
        return;
    }

    Cli_Disconnect(client);
    connected = false;
}

std::map<std::string, PlcValue> Snap7Test::read(const std::map<std::string, std::string>& tags) {
    std::map<std::string, PlcValue> results;

    for (const auto& [tagName, address] : tags) {
        int area, dbNumber, start, wordLen, size;
        PlcValueType type;
        parseAddress(address, area, dbNumber, start, wordLen, size, type);

        // Allocate buffer for the data
        std::vector<uint8_t> buffer(size);

        // Read the data
        int numElements = 1;
        if (type == PlcValueType::STRING) {
            numElements = size;
        } else if (type == PlcValueType::WSTRING) {
            numElements = size;
        }
        int result = Cli_ReadArea(client, area, dbNumber, start, numElements, wordLen, buffer.data());
        if (result != 0) {
            char errorText[1024];
            Cli_ErrorText(result, errorText, sizeof(errorText));
            throw std::runtime_error("Failed to read from PLC: " + std::string(errorText));
        }

        // Convert the data to the appropriate type
        switch (type) {
            case PlcValueType::BOOL:
                results[tagName] = PlcValue((buffer[0] & 0x01) != 0);
                break;
            case PlcValueType::SINT:
                results[tagName] = PlcValue(static_cast<int8_t>(buffer[0]));
                break;
            case PlcValueType::INT:
                results[tagName] = PlcValue(static_cast<int16_t>((buffer[0] << 8) | buffer[1]));
                break;
            case PlcValueType::DINT:
                results[tagName] = PlcValue(static_cast<int32_t>((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]));
                break;
            case PlcValueType::BYTE:
            case PlcValueType::USINT:
                results[tagName] = PlcValue(static_cast<uint8_t>(buffer[0]));
                break;
            case PlcValueType::WORD:
            case PlcValueType::UINT:
                results[tagName] = PlcValue(static_cast<uint16_t>((buffer[0] << 8) | buffer[1]));
                break;
            case PlcValueType::DWORD:
            case PlcValueType::UDINT:
                results[tagName] = PlcValue(static_cast<uint32_t>((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]));
                break;
            case PlcValueType::REAL:
                {
                    float value;
                    uint32_t temp;
                    memcpy(&temp, buffer.data(), sizeof(float));

                    // Byte-swap if needed (assuming buffer is big-endian)
                    temp = ((temp & 0x000000FF) << 24) |
                           ((temp & 0x0000FF00) << 8)  |
                           ((temp & 0x00FF0000) >> 8)  |
                           ((temp & 0xFF000000) >> 24);

                    memcpy(&value, &temp, sizeof(float));

                    results[tagName] = PlcValue(value);
                }
                break;
            case PlcValueType::LREAL:
                {
                    double value;
                    uint64_t temp;
                    memcpy(&temp, buffer.data(), sizeof(double));

                    // Swap bytes from big-endian to little-endian
                    temp = ((temp & 0x00000000000000FFULL) << 56) |
                           ((temp & 0x000000000000FF00ULL) << 40) |
                           ((temp & 0x0000000000FF0000ULL) << 24) |
                           ((temp & 0x00000000FF000000ULL) << 8)  |
                           ((temp & 0x000000FF00000000ULL) >> 8)  |
                           ((temp & 0x0000FF0000000000ULL) >> 24) |
                           ((temp & 0x00FF000000000000ULL) >> 40) |
                           ((temp & 0xFF00000000000000ULL) >> 56);

                    memcpy(&value, &temp, sizeof(double));

                    results[tagName] = PlcValue(value);
                }
                break;
            case PlcValueType::CHAR: {
                char value;
                memcpy(&value, buffer.data(), sizeof(char));
                results[tagName] = PlcValue(value);
                break;
            }
            case PlcValueType::WCHAR: {
                char16_t value;
                const uint8_t* data = buffer.data();

                // Convert from big-endian to host endian
                value = static_cast<char16_t>((data[0] << 8) | data[1]);

                results[tagName] = PlcValue(value);
                break;
            }
            case PlcValueType::STRING:
                {
                    // First byte is the max length, second byte is the actual length
                    int length = buffer[1];
                    std::string value(reinterpret_cast<char*>(buffer.data() + 2), length);
                    results[tagName] = PlcValue(value);
                }
                break;
            case PlcValueType::WSTRING:
                {
                    const uint8_t* data = buffer.data();

                    // First byte is the max length, second byte is the actual length
                    int length = (buffer[2] << 8) | buffer[3];

                    std::u16string value;
                    value.reserve(length);

                    // Starting from byte 4, read `length` UTF-16 code units
                    for (int i = 0; i < length; ++i) {
                        char16_t ch = (data[4 + i * 2] << 8) | data[4 + i * 2 + 1];
                        value.push_back(ch);
                    }

                    results[tagName] = PlcValue(value);
                }
                break;
            case PlcValueType::TIME:
                {
                    uint32_t milliseconds = static_cast<uint32_t>(buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3]);
                    double seconds = static_cast<double>(milliseconds) / 1000;
                    std::chrono::duration<double> durationInSeconds(seconds);
                    results[tagName] = PlcValue(durationInSeconds);
                }
                break;
            case PlcValueType::DATE:
                {
                    // DATE values are represented as 4-byte integers containing "days since 01.01.1980"
                    uint32_t days = static_cast<uint32_t>(buffer[0] << 8 | buffer[1]);

                    // Base date: January 1, 1980
                    int baseYear = 1990;
                    int baseMonth = 1;
                    int baseDay = 1;

                    // Add days to the base date
                    // This is a simple implementation that doesn't account for leap years correctly
                    // For a production implementation, a more robust date calculation would be needed
                    int year = baseYear;
                    int month = baseMonth;
                    int day = baseDay + days;

                    // Adjust day, month, year
                    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

                    while (true) {
                        // Check for leap year
                        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
                            daysInMonth[2] = 29;
                        } else {
                            daysInMonth[2] = 28;
                        }

                        if (day <= daysInMonth[month]) {
                            break;
                        }

                        day -= daysInMonth[month];
                        month++;

                        if (month > 12) {
                            month = 1;
                            year++;
                        }
                    }

                    results[tagName] = PlcValue(PlcDate(year, month, day));
                }
                break;
            case PlcValueType::TIME_OF_DAY:
                {
                    // TIME_OF_DAY values are represented as 4-byte unsigned integers containing milliseconds since midnight
                    uint32_t millisecondsSinceMidnight = static_cast<uint32_t>((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);

                    // Calculate hours, minutes, seconds, and milliseconds
                    int hours = millisecondsSinceMidnight / (60 * 60 * 1000);
                    int minutes = (millisecondsSinceMidnight % (60 * 60 * 1000)) / (60 * 1000);
                    int seconds = (millisecondsSinceMidnight % (60 * 1000)) / 1000;
                    int milliseconds = millisecondsSinceMidnight % 1000;

                    results[tagName] = PlcValue(PlcTimeOfDay(hours, minutes, seconds, milliseconds));
                }
                break;
            default:
                throw std::runtime_error("Unsupported word length: " + std::to_string(wordLen));
        }
    }

    return results;
}

void Snap7Test::parseAddress(const std::string& address, int& area, int& dbNumber, int& start, int& wordLen, int& size, PlcValueType& type) {
    // Parse address in the format "DB<db_number>.<byte_offset>.<bit_offset>.<data_type>"
    std::regex dbPattern(R"(%DB(\d+):(\d+)(?:\.(\d+))?:(\w+)(?:\((\d+)\))?)");
    std::smatch matches;

    if (std::regex_match(address, matches, dbPattern)) {
        dbNumber = std::stoi(matches[1].str());
        start = std::stoi(matches[2].str());
        int bitOffset = matches[3].matched ? std::stoi(matches[3].str()) : 0;
        std::string dataType = matches[4].str();
        int stringLength = matches[5].matched ? std::stoi(matches[5].str()) : 0;

        area = S7AreaDB;

        if (dataType == "BOOL") {
            type = PlcValueType::BOOL;
            wordLen = S7WLBit;
            size = 1;
            start = start * 8 + bitOffset; // Convert byte offset and bit offset to bit offset
        } else if (dataType == "BYTE") {
            type = PlcValueType::BYTE;
            wordLen = S7WLByte;
            size = 1;
        } else if (dataType == "SINT") {
            type = PlcValueType::SINT;
            wordLen = S7WLByte;
            size = 1;
        } else if (dataType == "USINT") {
            type = PlcValueType::USINT;
            wordLen = S7WLByte;
            size = 1;
        } else if (dataType == "WORD") {
            type = PlcValueType::WORD;
            wordLen = S7WLWord;
            size = 2;
        } else if (dataType == "DWORD") {
            type = PlcValueType::DWORD;
            wordLen = S7WLDWord;
            size = 4;
        } else if (dataType == "INT") {
            type = PlcValueType::INT;
            wordLen = S7WLWord;
            size = 2;
        } else if (dataType == "UINT") {
            type = PlcValueType::UINT;
            wordLen = S7WLWord;
            size = 2;
        } else if (dataType == "DINT") {
            type = PlcValueType::DINT;
            wordLen = S7WLDWord;
            size = 4;
        } else if (dataType == "UDINT") {
            type = PlcValueType::UDINT;
            wordLen = S7WLDWord;
            size = 4;
        } else if (dataType == "REAL") {
            type = PlcValueType::REAL;
            wordLen = S7WLReal;
            size = 4;
        } else if (dataType == "LREAL") {
            type = PlcValueType::LREAL;
            wordLen = S7WLReal;
            size = 8;
        } else if (dataType == "CHAR") {
            type = PlcValueType::CHAR;
            wordLen = S7WLChar;
            size = 1;
        } else if (dataType == "WCHAR") {
            type = PlcValueType::WCHAR;
            wordLen = S7WLWord;
            size = 2;
        } else if (dataType == "DATE") {
            type = PlcValueType::DATE;
            wordLen = S7WLWord;
            size = 2;
        } else if (dataType == "STRING") {
            type = PlcValueType::STRING;
            wordLen = S7WLByte;
            if (stringLength > 0) {
                size = stringLength + 2;
            } else {
                size = 254 + 2;
            }
        } else if (dataType == "WSTRING") {
            type = PlcValueType::WSTRING;
            wordLen = S7WLByte;
            if (stringLength > 0) {
                size = (stringLength * 2) + 4;
            } else {
                size = (254 * 2) * 4;
            }
        } else if (dataType == "TIME") {
            type = PlcValueType::TIME;
            wordLen = S7WLDWord;
            size = 4;
        } else if (dataType == "TIME_OF_DAY") {
            type = PlcValueType::TIME_OF_DAY;
            wordLen = S7WLDWord;
            size = 4;
        } else {
            throw std::runtime_error("Unsupported data type: " + dataType);
        }
    } else {
        // Parse address in the format "I<byte_offset>.<bit_offset>.<data_type>" (inputs)
        std::regex inputPattern("I(\\d+)(?:\\.(\\d+))?\\.(\\w+)");
        if (std::regex_match(address, matches, inputPattern)) {
            area = S7AreaPE;
            dbNumber = 0;
            start = std::stoi(matches[1].str());
            int bitOffset = matches[2].matched ? std::stoi(matches[2].str()) : 0;
            std::string dataType = matches[3].str();

            // Parse data type (similar to DB)
            if (dataType == "BOOL") {
                wordLen = S7WLBit;
                size = 1;
                start = start * 8 + bitOffset;
            } else if (dataType == "BYTE") {
                wordLen = S7WLByte;
                size = 1;
            } else if (dataType == "WORD") {
                wordLen = S7WLWord;
                size = 2;
            } else if (dataType == "DWORD") {
                wordLen = S7WLDWord;
                size = 4;
            } else {
                throw std::runtime_error("Unsupported data type: " + dataType);
            }
        } else {
            // Parse address in the format "Q<byte_offset>.<bit_offset>.<data_type>" (outputs)
            std::regex outputPattern("Q(\\d+)(?:\\.(\\d+))?\\.(\\w+)");
            if (std::regex_match(address, matches, outputPattern)) {
                area = S7AreaPA;
                dbNumber = 0;
                start = std::stoi(matches[1].str());
                int bitOffset = matches[2].matched ? std::stoi(matches[2].str()) : 0;
                std::string dataType = matches[3].str();

                // Parse data type (similar to DB)
                if (dataType == "BOOL") {
                    wordLen = S7WLBit;
                    size = 1;
                    start = start * 8 + bitOffset;
                } else if (dataType == "BYTE") {
                    wordLen = S7WLByte;
                    size = 1;
                } else if (dataType == "WORD") {
                    wordLen = S7WLWord;
                    size = 2;
                } else if (dataType == "DWORD") {
                    wordLen = S7WLDWord;
                    size = 4;
                } else {
                    throw std::runtime_error("Unsupported data type: " + dataType);
                }
            } else {
                // Parse address in the format "M<byte_offset>.<bit_offset>.<data_type>" (memory)
                std::regex memoryPattern("M(\\d+)(?:\\.(\\d+))?\\.(\\w+)");
                if (std::regex_match(address, matches, memoryPattern)) {
                    area = S7AreaMK;
                    dbNumber = 0;
                    start = std::stoi(matches[1].str());
                    int bitOffset = matches[2].matched ? std::stoi(matches[2].str()) : 0;
                    std::string dataType = matches[3].str();

                    // Parse data type (similar to DB)
                    if (dataType == "BOOL") {
                        wordLen = S7WLBit;
                        size = 1;
                        start = start * 8 + bitOffset;
                    } else if (dataType == "BYTE") {
                        wordLen = S7WLByte;
                        size = 1;
                    } else if (dataType == "WORD") {
                        wordLen = S7WLWord;
                        size = 2;
                    } else if (dataType == "DWORD") {
                        wordLen = S7WLDWord;
                        size = 4;
                    } else {
                        throw std::runtime_error("Unsupported data type: " + dataType);
                    }
                } else {
                    throw std::runtime_error("Invalid address format: " + address);
                }
            }
        }
    }
}
