#pragma once

#include <string>
#include <iostream>

struct ProgramOptions {
    std::string ssid;
    std::string dictionaryPath;
    int timeout = 5000; // Default timeout in milliseconds
    int threadCount = 4; // Default thread count
    bool showHelp = false;
    bool listNetworks = false;
};

class CommandLineParser {
public:
    static ProgramOptions parse(int argc, char* argv[]);
    static void printHelp();
    static void printVersion();
};
