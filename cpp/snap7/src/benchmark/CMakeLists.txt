# Benchmark test for snap7
CMAKE_MINIMUM_REQUIRED(VERSION 3.31.0)

# Add the benchmark executable
ADD_EXECUTABLE(s7_benchmark
    S7BenchmarkTest.cpp
    BaseTest.cpp
    Snap7Test.cpp
    PlcValue.cpp
)

# Link against the snap7 library
TARGET_LINK_LIBRARIES(s7_benchmark snap7)

# Install the benchmark executable
INSTALL(TARGETS s7_benchmark
    RUNTIME DESTINATION bin
)
