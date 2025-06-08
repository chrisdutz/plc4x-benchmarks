#include "Snap7OptimizedTest.h"
#include <regex>
#include <cstring>

Snap7OptimizedTest::Snap7OptimizedTest(const std::string& host, int rack, int slot)
    : host(host), rack(rack), slot(slot), client(0), connected(false), pduSize(0) {
}

Snap7OptimizedTest::~Snap7OptimizedTest() {
    if (connected) {
        disconnect();
    }
    if (client) {
        Cli_Destroy(client);
    }
}

std::string Snap7OptimizedTest::getName() {
    return "Snap7-Optimized";
}

void Snap7OptimizedTest::connect() {
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

    // Get the negotiated PDU size
    int requestedPduSize, negotiatedPduSize;
    result = Cli_GetPduLength(client, requestedPduSize, negotiatedPduSize);
    if (result != 0) {
        char errorText[1024];
        Cli_ErrorText(result, errorText, sizeof(errorText));
        throw std::runtime_error("Failed to get PDU size: " + std::string(errorText));
    }
    pduSize = negotiatedPduSize;

    connected = true;
}

void Snap7OptimizedTest::disconnect() {
    if (!connected) {
        return;
    }

    Cli_Disconnect(client);
    connected = false;
}

std::map<std::string, PlcValue> Snap7OptimizedTest::read(const std::map<std::string, std::string>& tags) {
    std::map<std::string, PlcValue> results;

    // Parse all addresses first
    std::vector<std::tuple<std::string, int, int, int, int, int, PlcValueType>> parsedTags;
    for (const auto& [tagName, address] : tags) {
        int area, dbNumber, start, wordLen, size;
        PlcValueType type;
        parseAddress(address, area, dbNumber, start, wordLen, size, type);
        parsedTags.push_back(std::make_tuple(tagName, area, dbNumber, start, wordLen, size, type));
    }

    // Group tags by area and dbNumber (required by S7 protocol)
    std::map<std::pair<int, int>, std::vector<std::tuple<std::string, int, int, int, int, PlcValueType>>> areaGroups;
    for (const auto& [tagName, area, dbNumber, start, wordLen, size, type] : parsedTags) {
        areaGroups[std::make_pair(area, dbNumber)].push_back(std::make_tuple(tagName, start, wordLen, size, 0, type));
    }

    // Process each area group
    for (auto& [areaKey, items] : areaGroups) {
        auto [area, dbNumber] = areaKey;

        // Sort items by start address to optimize for contiguous reads
        std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
            return std::get<1>(a) < std::get<1>(b);
        });

        // Create optimal groups based on PDU size
        std::vector<std::vector<std::tuple<std::string, int, int, int, int, PlcValueType>>> optimalGroups;
        std::vector<std::tuple<std::string, int, int, int, int, PlcValueType>> currentGroup;
        int currentGroupSize = 0;

        // Reserve space for PDU header (estimated at 14 bytes)
        int pduHeaderSize = 14;
        int maxItemsPerRequest = 20; // S7 protocol limit

        for (const auto& item : items) {
            auto [tagName, start, wordLen, size, _, type] = item;

            // Calculate the size this item would add to the PDU
            int itemSize = calculatePduItemSize(area, wordLen, size);

            // Check if adding this item would exceed PDU size or max items limit
            if ((currentGroup.size() >= maxItemsPerRequest) || 
                (currentGroupSize + itemSize + pduHeaderSize > pduSize && !currentGroup.empty())) {
                // Current group is full, start a new one
                optimalGroups.push_back(currentGroup);
                currentGroup.clear();
                currentGroupSize = 0;
            }

            // Add item to current group
            currentGroup.push_back(item);
            currentGroupSize += itemSize;
        }

        // Add the last group if not empty
        if (!currentGroup.empty()) {
            optimalGroups.push_back(currentGroup);
        }

        // Process each optimal group
        for (const auto& group : optimalGroups) {
            // If there's only one item in the group, use the original method
            if (group.size() == 1) {
                auto [tagName, start, wordLen, size, _, type] = group[0];

                // Allocate buffer for the data
                std::vector<uint8_t> buffer(size);

                // Read the data
                int numElements = 1;
                if (type == PlcValueType::STRING || type == PlcValueType::WSTRING) {
                    numElements = size;
                }

                int result = Cli_ReadArea(client, area, dbNumber, start, numElements, wordLen, buffer.data());
                if (result != 0) {
                    char errorText[1024];
                    Cli_ErrorText(result, errorText, sizeof(errorText));
                    throw std::runtime_error("Failed to read from PLC: " + std::string(errorText));
                }

                // Convert the data to the appropriate type and store in results
                results[tagName] = convertBufferToPlcValue(buffer.data(), type);
            } else {
                // Use multi-item read for groups with more than one item
                std::vector<TS7DataItem> dataItems(group.size());
                std::vector<std::vector<uint8_t>> buffers(group.size());

                // Prepare data items and buffers
                for (size_t i = 0; i < group.size(); i++) {
                    auto [tagName, start, wordLen, size, _, type] = group[i];

                    // Allocate buffer for this item
                    buffers[i].resize(size);

                    // Set up data item
                    dataItems[i].Area = area;
                    dataItems[i].WordLen = wordLen;
                    dataItems[i].DBNumber = dbNumber;
                    dataItems[i].Start = start;
                    dataItems[i].Amount = (type == PlcValueType::STRING || type == PlcValueType::WSTRING) ? size : 1;
                    dataItems[i].pdata = buffers[i].data();
                }

                // Perform multi-item read
                int result = Cli_ReadMultiVars(client, dataItems.data(), static_cast<int>(dataItems.size()));
                if (result != 0) {
                    char errorText[1024];
                    Cli_ErrorText(result, errorText, sizeof(errorText));
                    throw std::runtime_error("Failed to read multiple items from PLC: " + std::string(errorText));
                }

                // Process results
                for (size_t i = 0; i < group.size(); i++) {
                    auto [tagName, start, wordLen, size, _, type] = group[i];

                    // Check if this specific item had an error
                    if (dataItems[i].Result != 0) {
                        char errorText[1024];
                        Cli_ErrorText(dataItems[i].Result, errorText, sizeof(errorText));
                        throw std::runtime_error("Failed to read item " + tagName + " from PLC: " + std::string(errorText));
                    }

                    // Convert the data to the appropriate type and store in results
                    results[tagName] = convertBufferToPlcValue(buffers[i].data(), type);
                }
            }
        }
    }

    return results;
}

int Snap7OptimizedTest::calculatePduItemSize(int area, int wordLen, int size) {
    // Each item in the PDU consists of:
    // - 12 bytes for the request header (TReqFunReadItem)
    // - The actual data size

    // For strings and wstrings, the amount is the size in bytes
    // For other types, the amount is the number of elements
    int amount = 1;
    if (wordLen == S7WLByte) {
        // For strings and wstrings, the amount is the size in bytes
        amount = size;
    }

    // The header size is fixed at 12 bytes
    int headerSize = 12;

    // The data size depends on the word length and amount
    int dataSize = 0;
    switch (wordLen) {
        case S7WLBit:
            dataSize = 1; // 1 bit
            break;
        case S7WLByte:
            dataSize = amount; // 1 byte per element
            break;
        case S7WLWord:
            dataSize = amount * 2; // 2 bytes per element
            break;
        case S7WLDWord:
            dataSize = amount * 4; // 4 bytes per element
            break;
        case S7WLReal:
            dataSize = amount * 4; // 4 bytes per element (float)
            break;
        case S7WLCounter:
        case S7WLTimer:
            dataSize = amount * 2; // 2 bytes per element
            break;
        default:
            dataSize = size; // Use the provided size
    }

    return headerSize + dataSize;
}

PlcValue Snap7OptimizedTest::convertBufferToPlcValue(void* buffer, PlcValueType type) {
    uint8_t* data = static_cast<uint8_t*>(buffer);

    // Convert the data to the appropriate type
    switch (type) {
        case PlcValueType::BOOL:
            return PlcValue((data[0] & 0x01) != 0);
        case PlcValueType::SINT:
            return PlcValue(static_cast<int8_t>(data[0]));
        case PlcValueType::INT:
            return PlcValue(static_cast<int16_t>((data[0] << 8) | data[1]));
        case PlcValueType::DINT:
            return PlcValue(static_cast<int32_t>((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]));
        case PlcValueType::BYTE:
        case PlcValueType::USINT:
            return PlcValue(static_cast<uint8_t>(data[0]));
        case PlcValueType::WORD:
        case PlcValueType::UINT:
            return PlcValue(static_cast<uint16_t>((data[0] << 8) | data[1]));
        case PlcValueType::DWORD:
        case PlcValueType::UDINT:
            return PlcValue(static_cast<uint32_t>((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]));
        case PlcValueType::REAL:
            {
                float value;
                uint32_t temp;
                memcpy(&temp, data, sizeof(float));

                // Byte-swap if needed (assuming buffer is big-endian)
                temp = ((temp & 0x000000FF) << 24) |
                       ((temp & 0x0000FF00) << 8)  |
                       ((temp & 0x00FF0000) >> 8)  |
                       ((temp & 0xFF000000) >> 24);

                memcpy(&value, &temp, sizeof(float));

                return PlcValue(value);
            }
        case PlcValueType::LREAL:
            {
                double value;
                uint64_t temp;
                memcpy(&temp, data, sizeof(double));

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

                return PlcValue(value);
            }
        case PlcValueType::CHAR: 
            {
                char value;
                memcpy(&value, data, sizeof(char));
                return PlcValue(value);
            }
        case PlcValueType::WCHAR: 
            {
                char16_t value;
                // Convert from big-endian to host endian
                value = static_cast<char16_t>((data[0] << 8) | data[1]);
                return PlcValue(value);
            }
        case PlcValueType::STRING:
            {
                // First byte is the max length, second byte is the actual length
                int length = data[1];
                std::string value(reinterpret_cast<char*>(data + 2), length);
                return PlcValue(value);
            }
        case PlcValueType::WSTRING:
            {
                // First byte is the max length, second byte is the actual length
                int length = (data[2] << 8) | data[3];

                std::u16string value;
                value.reserve(length);

                // Starting from byte 4, read `length` UTF-16 code units
                for (int i = 0; i < length; ++i) {
                    char16_t ch = (data[4 + i * 2] << 8) | data[4 + i * 2 + 1];
                    value.push_back(ch);
                }

                return PlcValue(value);
            }
        case PlcValueType::TIME:
            {
                uint32_t milliseconds = static_cast<uint32_t>(data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]);
                double seconds = static_cast<double>(milliseconds) / 1000;
                std::chrono::duration<double> durationInSeconds(seconds);
                return PlcValue(durationInSeconds);
            }
        case PlcValueType::DATE:
            {
                // DATE values are represented as 4-byte integers containing "days since 01.01.1980"
                uint32_t days = static_cast<uint32_t>(data[0] << 8 | data[1]);

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

                return PlcValue(PlcDate(year, month, day));
            }
        case PlcValueType::TIME_OF_DAY:
            {
                // TIME_OF_DAY values are represented as 4-byte unsigned integers containing milliseconds since midnight
                uint32_t millisecondsSinceMidnight = static_cast<uint32_t>((data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3]);

                // Calculate hours, minutes, seconds, and milliseconds
                int hours = millisecondsSinceMidnight / (60 * 60 * 1000);
                int minutes = (millisecondsSinceMidnight % (60 * 60 * 1000)) / (60 * 1000);
                int seconds = (millisecondsSinceMidnight % (60 * 1000)) / 1000;
                int milliseconds = millisecondsSinceMidnight % 1000;

                return PlcValue(PlcTimeOfDay(hours, minutes, seconds, milliseconds));
            }
        default:
            throw std::runtime_error("Unsupported data type: " + std::to_string(static_cast<int>(type)));
    }
}

void Snap7OptimizedTest::parseAddress(const std::string& address, int& area, int& dbNumber, int& start, int& wordLen, int& size, PlcValueType& type) {
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
