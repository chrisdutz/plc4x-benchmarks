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
        int result = Cli_ReadArea(client, area, dbNumber, start, 1, wordLen, buffer.data());
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
            /*case S7WLString:
                {
                    // First byte is the max length, second byte is the actual length
                    int length = buffer[1];
                    std::string value(reinterpret_cast<char*>(buffer.data() + 2), length);
                    results[tagName] = PlcValue(value);
                }
                break;*/
            default:
                throw std::runtime_error("Unsupported word length: " + std::to_string(wordLen));
        }
    }

    return results;
}

void Snap7Test::parseAddress(const std::string& address, int& area, int& dbNumber, int& start, int& wordLen, int& size, PlcValueType& type) {
    // Parse address in the format "DB<db_number>.<byte_offset>.<bit_offset>.<data_type>"
    std::regex dbPattern("%DB(\\d+)\\:(\\d+)(?:\\.(\\d+))?:(\\w+)");
    std::smatch matches;

    if (std::regex_match(address, matches, dbPattern)) {
        dbNumber = std::stoi(matches[1].str());
        start = std::stoi(matches[2].str());
        int bitOffset = matches[3].matched ? std::stoi(matches[3].str()) : 0;
        std::string dataType = matches[4].str();

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
            /*} else if (dataType.substr(0, 6) == "STRING") {
            wordLen = S7WLString;
            // Extract the string length from STRING[<length>]
            std::regex stringPattern("STRING\\[(\\d+)\\]");
            std::smatch stringMatches;
            if (std::regex_match(dataType, stringMatches, stringPattern)) {
                int stringLength = std::stoi(stringMatches[1].str());
                size = stringLength + 2; // 2 bytes for max length and actual length
            } else {
                size = 256; // Default size
            }*/
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
