#pragma once

#include <string>
#include <vector>

struct SavedServerEntry {
    std::string name;
    std::string address;
    int port;
};

class SavedServerList {
public:
    static std::vector<SavedServerEntry> load();
    static bool addOrUpdate(const SavedServerEntry& entry);
    static bool remove(const std::string& address, int port);

private:
    static bool save(const std::vector<SavedServerEntry>& entries);
    static std::string getSettingsPath();
    static std::string trim(const std::string& value);
    static std::string sanitizeAddress(const std::string& value);
    static std::string sanitizeName(const std::string& value, const std::string& fallbackAddress);
    static int sanitizePort(int value);
};
