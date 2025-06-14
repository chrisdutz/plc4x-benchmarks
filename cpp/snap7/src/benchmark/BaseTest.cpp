#include "BaseTest.h"

#include <codecvt>
#include <regex>

TestResults BaseTest::run(int numCycles, int cycleTime, const std::map<std::string, std::string>& tagValues) {
    // Prepare the input and expected output maps
    std::map<std::string, std::string> tags;
    std::map<std::string, PlcValue> expectedResults;
    int tagNumber = 0;

    for (const auto& [address, valueString] : tagValues) {
        tagNumber++;
        std::string tagName = "tag-" + std::to_string(tagNumber);
        tags[tagName] = address;
        expectedResults[tagName] = getValue(valueString);
    }

    // Execute the read operation
    int connectionTime = 0;
    int disconnectionTime = 0;
    std::vector<int> readTimes(numCycles, 0);

    try {
        // Connect to the PLC
        auto startTime = std::chrono::high_resolution_clock::now();
        connect();
        auto endTime = std::chrono::high_resolution_clock::now();
        connectionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        // Perform the read operations
        for (int i = 0; i < numCycles; i++) {
            // Read the values
            startTime = std::chrono::high_resolution_clock::now();
            std::map<std::string, PlcValue> results = read(tags);
            endTime = std::chrono::high_resolution_clock::now();
            int readTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            readTimes[i] = readTime;

            // Check the results
            for (const auto& [tagName, value] : results) {
                if (expectedResults.find(tagName) == expectedResults.end()) {
                    throw std::runtime_error("Unexpected result: " + tagName + " for tag address " + tags[tagName]);
                }
                if (value != expectedResults[tagName]) {
                    throw std::runtime_error("Unexpected result: " + tagName + " for tag address " + tags[tagName]);
                }
            }

            // Wait for the next cycle
            if (i < numCycles - 1) {
                std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error during test: " + std::string(e.what()));
    }

    // Disconnect from the PLC
    auto startTime = std::chrono::high_resolution_clock::now();
    disconnect();
    auto endTime = std::chrono::high_resolution_clock::now();
    disconnectionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    return TestResults(connectionTime, disconnectionTime, numCycles, readTimes);
}

PlcValue BaseTest::getValue(const std::string& value) {
    std::string typeString = value.substr(0, value.find(';'));
    std::string valueString = value.substr(value.find(';') + 1);

    if (typeString == "BOOL") {
        return PlcValue(valueString == "true");
    }

    if (typeString == "BYTE") {
        return PlcValue(static_cast<int8_t>(std::stoi(valueString)));
    }
    if (typeString == "WORD") {
        return PlcValue(static_cast<int16_t>(std::stoi(valueString)));
    }
    if (typeString == "DWORD") {
        return PlcValue(std::stoi(valueString));
    }
    if (typeString == "LWORD") {
        return PlcValue(std::stoi(valueString));
    }

    if (typeString == "SINT") {
        return PlcValue(static_cast<int8_t>(std::stoi(valueString)));
    }
    if (typeString == "INT") {
        return PlcValue(static_cast<int16_t>(std::stoi(valueString)));
    }
    if (typeString == "DINT") {
        return PlcValue(std::stoi(valueString));
    }
    if (typeString == "LINT") {
        return PlcValue(std::stoi(valueString));
    }

    if (typeString == "USINT") {
        return PlcValue(static_cast<uint8_t>(std::stoi(valueString)));
    }
    if (typeString == "UINT") {
        return PlcValue(static_cast<uint16_t>(std::stoi(valueString)));
    }
    if (typeString == "UDINT") {
        return PlcValue(static_cast<uint32_t>(std::stoll(valueString)));
    }
    if (typeString == "ULINT") {
        return PlcValue(static_cast<uint64_t>(std::stoll(valueString)));
    }

    if (typeString == "REAL") {
        return PlcValue(std::stof(valueString));
    }
    if (typeString == "LREAL") {
        return PlcValue(std::stod(valueString));
    }

    if (typeString == "CHAR") {
        return PlcValue(*(valueString.c_str()));
    }
    if (typeString == "WCHAR") {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        std::u16string u16 = convert.from_bytes(valueString);

        if (u16.empty()) {
            throw std::runtime_error("WCHAR input is empty or invalid UTF-8");
        }

        return PlcValue(u16[0]);
    }

    if (typeString == "STRING") {
        return PlcValue(valueString);
    }
    if (typeString == "WSTRING") {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
        std::u16string u16 = convert.from_bytes(valueString);
        return PlcValue(u16);
    }

    if (typeString == "TIME") {
        std::regex re(R"(PT(\d+(?:\.\d+)?)S)");
        std::smatch match;
        if (std::regex_match(valueString, match, re)) {
            double seconds = std::stod(match[1].str());
            return PlcValue(std::chrono::duration<double>(seconds));
        }
        throw std::invalid_argument("Invalid duration format");
    }
    if (typeString == "LTIME") {
        // TODO: Implement
    }
    if (typeString == "DATE") {
        // Parse date in format "YYYY-MM-DD"
        std::regex dateRegex(R"(\d{4}-\d{2}-\d{2})");
        if (!std::regex_match(valueString, dateRegex)) {
            throw std::invalid_argument("Invalid date format. Expected YYYY-MM-DD");
        }

        int year = std::stoi(valueString.substr(0, 4));
        int month = std::stoi(valueString.substr(5, 2));
        int day = std::stoi(valueString.substr(8, 2));

        return PlcValue(PlcDate(year, month, day));
    }
    if (typeString == "LDATE") {
        // TODO: Implement
    }
    if (typeString == "TIME_OF_DAY") {
        // Parse time of day in format "HH:MM:SS.mmm"
        std::regex timeRegex(R"((\d{2}):(\d{2}):(\d{2})(?:\.(\d{1,3}))?)");
        std::smatch match;

        if (!std::regex_match(valueString, match, timeRegex)) {
            throw std::invalid_argument("Invalid time of day format. Expected HH:MM:SS.mmm");
        }

        int hour = std::stoi(match[1].str());
        int minute = std::stoi(match[2].str());
        int second = std::stoi(match[3].str());
        int millisecond = match[4].matched ? std::stoi(match[4].str()) : 0;

        // Pad milliseconds if needed (e.g., if .12 was provided, it means 120 milliseconds)
        if (match[4].matched && match[4].length() < 3) {
            millisecond *= pow(10, 3 - match[4].length());
        }

        // Validate time components
        if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || 
            second < 0 || second > 59 || millisecond < 0 || millisecond > 999) {
            throw std::invalid_argument("Invalid time of day values");
        }

        return PlcValue(PlcTimeOfDay(hour, minute, second, millisecond));
    }
    if (typeString == "LTIME_OF_DAY") {
        // TODO: Implement
    }
    if (typeString == "DATE_AND_TIME") {
        // TODO: Implement
    }
    if (typeString == "DATE_AND_LTIME") {
        // TODO: Implement
    }
    if (typeString == "LDATE_AND_TIME") {
        // TODO: Implement
    }
    if (typeString == "RAW_BYTE_ARRAY") {
        // TODO: Implement
    }
    throw std::runtime_error("Unknown type: " + typeString);
}
