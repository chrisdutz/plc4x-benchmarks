#include "Snap7Test.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

/**
 * Run a benchmark test.
 * 
 * @param test The test to run
 * @param numCycles Number of read cycles to perform
 * @param cycleTime Time between read cycles (in milliseconds)
 * @param tagValues Map of tag addresses to expected values
 */
void runTest(BaseTest& test, int numCycles, int cycleTime, const std::map<std::string, std::string>& tagValues) {
    std::cout << "Running: '" << test.getName() << "'" << std::endl;
    TestResults testResults = test.run(numCycles, cycleTime, tagValues);
    
    int totalReadTime = 0;
    for (int readTime : testResults.readTimes) {
        totalReadTime += readTime;
    }
    int averageReadTime = totalReadTime / testResults.numReadCycles;
    
    std::cout << "  --> " << testResults.connectionTime << " ms connect, "
              << testResults.disconnectionTime << " ms disconnect, "
              << averageReadTime << " ms avg read time" << std::endl;
}

/**
 * Main function.
 */
int main(int argc, char* argv[]) {
    // Get configuration from environment variables or command line arguments
    std::string host = std::getenv("host") ? std::getenv("host") : "192.168.23.30";
    int remoteRack = std::getenv("remoteRack") ? std::stoi(std::getenv("remoteRack")) : 0;
    int remoteSlot = std::getenv("remoteSlot") ? std::stoi(std::getenv("remoteSlot")) : 1;
    int numCycles = std::getenv("numCycles") ? std::stoi(std::getenv("numCycles")) : 50;
    int cycleTime = std::getenv("cycleTime") ? std::stoi(std::getenv("cycleTime")) : 300;
    std::string defaultTags = "%DB4:0.0:BOOL|BOOL;true\n"
            "%DB4:1:BYTE|USINT;42\n"
            "%DB4:2:WORD|UINT;42424\n"
            "%DB4:4:DWORD|UDINT;4242442424\n"
            "%DB4:16:SINT|SINT;-42\n"
            "%DB4:17:USINT|USINT;42\n"
            "%DB4:18:INT|INT;-2424\n"
            "%DB4:20:UINT|UINT;42424\n"
            "%DB4:22:DINT|DINT;-242442424\n"
            "%DB4:26:UDINT|UDINT;4242442424\n"
            "%DB4:46:REAL|REAL;3.141593\n"
            // Is seems in C++ reals decode slightly different.
            //"%DB4:50:LREAL|LREAL;2.71828182846\n"
            "%DB4:50:LREAL|LREAL;2.7182807922363281\n"
            "%DB4:136:CHAR|CHAR;H\n"
            "%DB4:138:WCHAR|WCHAR;w\n"
            "%DB4:140:STRING(10)|STRING;hurz\n"
            "%DB4:396:WSTRING(10)|WSTRING;wolf\n"
            "%DB4:58:TIME|TIME;PT1.234S\n";
            /*"%DB4:70:DATE|DATE;1998-03-28\n"
            "%DB4:72:TIME_OF_DAY|TIME_OF_DAY;15:36:30.123\n";*/
    
    // Parse command line arguments (override environment variables)
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--remoteRack" && i + 1 < argc) {
            remoteRack = std::stoi(argv[++i]);
        } else if (arg == "--remoteSlot" && i + 1 < argc) {
            remoteSlot = std::stoi(argv[++i]);
        } else if (arg == "--numCycles" && i + 1 < argc) {
            numCycles = std::stoi(argv[++i]);
        } else if (arg == "--cycleTime" && i + 1 < argc) {
            cycleTime = std::stoi(argv[++i]);
        }
    }
    
    // Get tag values from environment variable or file
    std::map<std::string, std::string> tagValues;
    std::string tagStringList;
    
    if (std::getenv("tags")) {
        tagStringList = std::getenv("tags");
    } else if (std::getenv("tagsFile")) {
        std::string tagsFile = std::getenv("tagsFile");
        std::ifstream file(tagsFile);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            tagStringList = buffer.str();
            file.close();
        } else {
            std::cerr << "Failed to open tags file: " << tagsFile << std::endl;
            return 1;
        }
    } else {
        tagStringList = defaultTags;
    }
    
    std::istringstream iss(tagStringList);
    std::string line;
    while (std::getline(iss, line)) {
        size_t pos = line.find('|');
        if (pos == std::string::npos) {
            continue;
        }
        std::string address = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        address.erase(0, address.find_first_not_of(" \t"));
        address.erase(address.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        tagValues[address] = value;
    }
    
    std::cout << "Scenario: " << tagValues.size() << " tags, " << numCycles << " cycles, " << cycleTime << "ms intervals" << std::endl << std::endl;
    
    // Run the test
    Snap7Test snap7Test(host, remoteRack, remoteSlot);
    runTest(snap7Test, numCycles, cycleTime, tagValues);
    
    return 0;
}