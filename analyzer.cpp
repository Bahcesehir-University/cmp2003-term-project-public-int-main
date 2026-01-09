#include "analyzer.h"
#include <fstream>
#include <algorithm>
#include <cstring>

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    zoneId.clear();
    zoneNames.clear();
    zoneTotals.clear();
    zoneHours.clear();

    zoneId.reserve(200000);
    zoneNames.reserve(200000);
    zoneTotals.reserve(200000);
    zoneHours.reserve(200000);

    std::ifstream file(csvPath);
    if (!file.is_open()) return;

    std::string line;
    bool headerSkipped = false;

    while (std::getline(file, line)) {
        if (!headerSkipped) {
            headerSkipped = true;
            continue;
        }

        char* fields[6];
        char* p = line.data();
        fields[0] = p;

        int idx = 1;
        for (size_t i = 0; i < line.size() && idx < 6; ++i) {
            if (line[i] == ',') {
                line[i] = '\0';
                fields[idx++] = &line[i + 1];
            }
        }
        if (idx < 6) continue;

        const char* zone = fields[1];
        if (!zone || *zone == '\0') continue;
        if (std::strcmp(zone, "INVALID_ID") == 0) continue;

        const char* ts = fields[3];
        if (!ts || ts[12] == '\0') continue;
        if (ts[4] != '-' || ts[7] != '-' || ts[10] != ' ') continue;
        if (ts[11] < '0' || ts[11] > '9' ||
            ts[12] < '0' || ts[12] > '9') continue;

        int hour = (ts[11] - '0') * 10 + (ts[12] - '0');
        if (hour > 23) continue;

        auto it = zoneId.find(zone);
        int id;
        if (it == zoneId.end()) {
            id = (int)zoneNames.size();
            zoneId.emplace(zone, id);
            zoneNames.emplace_back(zone);
            zoneTotals.push_back(0);
            zoneHours.emplace_back(std::array<long long, 24>{});
        } else {
            id = it->second;
        }

        ++zoneTotals[id];
        ++zoneHours[id][hour];
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;
    result.reserve(zoneNames.size());

    for (size_t i = 0; i < zoneNames.size(); ++i) {
        result.push_back({zoneNames[i], zoneTotals[i]});
    }

    if ((int)result.size() > k) {
        std::nth_element(result.begin(), result.begin() + k, result.end(),
            [](const ZoneCount& a, const ZoneCount& b) {
                if (a.count != b.count) return a.count > b.count;
                return a.zone < b.zone;
            });
        result.resize(k);
    }

    std::sort(result.begin(), result.end(),
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) return a.count > b.count;
            return a.zone < b.zone;
        });

    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    result.reserve(512);

    for (size_t i = 0; i < zoneNames.size(); ++i) {
        const auto& hours = zoneHours[i];
        for (int h = 0; h < 24; ++h) {
            if (hours[h] > 0) {
                result.push_back({zoneNames[i], h, hours[h]});
            }
        }
    }

    if ((int)result.size() > k) {
        std::nth_element(result.begin(), result.begin() + k, result.end(),
            [](const SlotCount& a, const SlotCount& b) {
                if (a.count != b.count) return a.count > b.count;
                if (a.zone != b.zone) return a.zone < b.zone;
                return a.hour < b.hour;
            });
        result.resize(k);
    }

    std::sort(result.begin(), result.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) return a.count > b.count;
            if (a.zone != b.zone) return a.zone < b.zone;
            return a.hour < b.hour;
        });

    return result;
}

