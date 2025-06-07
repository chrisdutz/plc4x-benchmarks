#include "BaseTest.h"

#include <codecvt>

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
        return PlcValue(valueString);
    }

    if (typeString == "TIME") {

    }
    if (typeString == "LTIME") {

    }
    if (typeString == "DATE") {

    }
    if (typeString == "LDATE") {

    }
    if (typeString == "TIME_OF_DAY") {

    }
    if (typeString == "LTIME_OF_DAY") {

    }
    if (typeString == "DATE_AND_TIME") {

    }
    if (typeString == "DATE_AND_LTIME") {

    }
    if (typeString == "LDATE_AND_TIME") {

    }
    if (typeString == "RAW_BYTE_ARRAY") {

    }
    throw std::runtime_error("Unknown type: " + typeString);
}
