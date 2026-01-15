#pragma once

#include <string>
#include <vector>
#include <windows.h>
#include <wlanapi.h>
#pragma comment(lib, "wlanapi.lib")

class WiFiConnector {
private:
    HANDLE hClient;
    DWORD dwMaxClient;
    DWORD dwCurVersion;
    WLAN_INTERFACE_INFO_LIST* pIfList;
    WLAN_INTERFACE_INFO* pIfInfo;
    bool isInitialized;

public:
    WiFiConnector();
    ~WiFiConnector();

    bool initialize();
    bool connectToNetwork(const std::string& ssid, const std::string& password);
    bool disconnect();
    std::vector<std::string> getAvailableNetworks();
    bool isConnected() const;
    std::string getCurrentSSID() const;
};

class WiFiBruteForcer {
private:
    std::string targetSSID;
    std::string dictionaryPath;
    int timeout;
    int threadCount;
    bool foundPassword;
    std::string correctPassword;

public:
    WiFiBruteForcer(const std::string& ssid, const std::string& dictPath, int timeout = 5000, int threads = 4);
    bool start();
    void stop();
    std::string getFoundPassword() const;
    bool isPasswordFound() const;

private:
    void bruteForceThread(const std::vector<std::string>& passwords, int start, int end);
    bool testPassword(const std::string& password);
    void displayProgress(int tested, int total);
};
