#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

struct ZoneCount {
    std::string zone;
    long long count;
};

struct SlotCount {
    std::string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
private:
    std::unordered_map<std::string, int> zoneId;
    std::vector<std::string> zoneNames;

    std::vector<long long> zoneTotals;
    std::vector<std::array<long long, 24>> zoneHours;

public:
    void ingestFile(const std::string& csvPath);
    std::vector<ZoneCount> topZones(int k = 10) const;
    std::vector<SlotCount> topBusySlots(int k = 10) const;
};


