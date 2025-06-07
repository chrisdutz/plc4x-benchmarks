#ifndef TEST_RESULTS_H
#define TEST_RESULTS_H

#include <vector>

/**
 * Struct to hold the results of a benchmark test.
 */
struct TestResults {
    int connectionTime;       // Time taken to establish a connection to the PLC (in milliseconds)
    int disconnectionTime;    // Time taken to disconnect from the PLC (in milliseconds)
    int numReadCycles;        // Number of read cycles performed
    std::vector<int> readTimes; // Array of times taken for each read operation (in milliseconds)

    TestResults(int connectionTime, int disconnectionTime, int numReadCycles, const std::vector<int>& readTimes)
        : connectionTime(connectionTime), disconnectionTime(disconnectionTime), numReadCycles(numReadCycles), readTimes(readTimes) {}
};

#endif // TEST_RESULTS_H