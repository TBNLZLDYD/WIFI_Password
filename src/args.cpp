#include "args.h"
#include <string.h>

ProgramOptions CommandLineParser::parse(int argc, char* argv[]) {
    ProgramOptions options;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            options.showHelp = true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printVersion();
            exit(0);
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            options.listNetworks = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--ssid") == 0) {
            if (i + 1 < argc) {
                options.ssid = argv[++i];
            } else {
                std::cerr << "Error: -s/--ssid requires an argument" << std::endl;
                printHelp();
                exit(1);
            }
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dictionary") == 0) {
            if (i + 1 < argc) {
                options.dictionaryPath = argv[++i];
            } else {
                std::cerr << "Error: -d/--dictionary requires an argument" << std::endl;
                printHelp();
                exit(1);
            }
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--timeout") == 0) {
            if (i + 1 < argc) {
                try {
                    options.timeout = std::stoi(argv[++i]);
                    if (options.timeout <= 0) {
                        std::cerr << "Error: -t/--timeout must be a positive integer" << std::endl;
                        printHelp();
                        exit(1);
                    }
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: -t/--timeout must be an integer" << std::endl;
                    printHelp();
                    exit(1);
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: -t/--timeout value is too large" << std::endl;
                    printHelp();
                    exit(1);
                }
            } else {
                std::cerr << "Error: -t/--timeout requires an argument" << std::endl;
                printHelp();
                exit(1);
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--threads") == 0) {
            if (i + 1 < argc) {
                try {
                    options.threadCount = std::stoi(argv[++i]);
                    if (options.threadCount <= 0 || options.threadCount > 16) {
                        std::cerr << "Error: -c/--threads must be an integer between 1 and 16" << std::endl;
                        printHelp();
                        exit(1);
                    }
                } catch (const std::invalid_argument&) {
                    std::cerr << "Error: -c/--threads must be an integer" << std::endl;
                    printHelp();
                    exit(1);
                } catch (const std::out_of_range&) {
                    std::cerr << "Error: -c/--threads value is too large" << std::endl;
                    printHelp();
                    exit(1);
                }
            } else {
                std::cerr << "Error: -c/--threads requires an argument" << std::endl;
                printHelp();
                exit(1);
            }
        } else {
            std::cerr << "Error: Unknown option '" << argv[i] << "'" << std::endl;
            printHelp();
            exit(1);
        }
    }

    // Validate required options
    if (!options.showHelp && !options.listNetworks) {
        if (options.ssid.empty()) {
            std::cerr << "Error: SSID is required" << std::endl;
            printHelp();
            exit(1);
        }
        if (options.dictionaryPath.empty()) {
            std::cerr << "Error: Dictionary path is required" << std::endl;
            printHelp();
            exit(1);
        }
    }

    return options;
}

void CommandLineParser::printHelp() {
    std::cout << "WiFi Brute Forcer - Educational Research Tool" << std::endl;
    std::cout << "Usage: wifi_brute [options]" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help             Show this help message" << std::endl;
    std::cout << "  -v, --version          Show version information" << std::endl;
    std::cout << "  -l, --list             List available WiFi networks" << std::endl;
    std::cout << "  -s, --ssid <SSID>      Target WiFi SSID" << std::endl;
    std::cout << "  -d, --dictionary <path> Path to password dictionary file" << std::endl;
    std::cout << "  -t, --timeout <ms>     Connection timeout in milliseconds (default: 5000)" << std::endl;
    std::cout << "  -c, --threads <count>  Number of threads to use (default: 4)" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Example: wifi_brute -s MyWiFi -d passwords.txt -t 3000 -c 8" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "Note: This tool is for educational research purposes only." << std::endl;
    std::cout << "Ensure you have proper authorization before using it." << std::endl;
}

void CommandLineParser::printVersion() {
    std::cout << "WiFi Brute Forcer v1.0" << std::endl;
    std::cout << "Educational research tool for WiFi password testing" << std::endl;
    std::cout << "Copyright (c) 2026" << std::endl;
}
