#include "args.h"
#include "wifi.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // Print disclaimer
    std::cout << "====================================" << std::endl;
    std::cout << "WiFi Brute Forcer - Educational Research Tool" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "WARNING: This tool is for educational purposes only." << std::endl;
    std::cout << "Ensure you have proper authorization before use." << std::endl;
    std::cout << "Unauthorized use may violate laws." << std::endl;
    std::cout << "====================================" << std::endl << std::endl;

    // Parse command line arguments
    ProgramOptions options = CommandLineParser::parse(argc, argv);

    // Show help if requested
    if (options.showHelp) {
        CommandLineParser::printHelp();
        return 0;
    }

    // List available networks if requested
    if (options.listNetworks) {
        WiFiConnector connector;
        if (!connector.initialize()) {
            std::cerr << "Error: Could not initialize WiFi interface" << std::endl;
            return 1;
        }

        std::vector<std::string> networks = connector.getAvailableNetworks();
        if (networks.empty()) {
            std::cout << "No available WiFi networks found" << std::endl;
        } else {
            std::cout << "Available WiFi networks:" << std::endl;
            std::cout << "=========================" << std::endl;
            for (const auto& network : networks) {
                std::cout << "- " << network << std::endl;
            }
        }
        return 0;
    }

    // Start brute force attack
    WiFiBruteForcer bruteForcer(options.ssid, options.dictionaryPath, options.timeout, options.threadCount);
    bool success = bruteForcer.start();

    return success ? 0 : 1;
}
