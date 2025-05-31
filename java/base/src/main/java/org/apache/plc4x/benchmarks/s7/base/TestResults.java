package org.apache.plc4x.benchmarks.s7.base;

public record TestResults (int connectionTime, int disconnectionTime, int numReadCycles, int[] readTimes) {}
