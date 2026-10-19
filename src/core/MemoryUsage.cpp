#include "core/MemoryUsage.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

size_t getCurrentMemoryUsageKB() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024;
    }
    return 0;
}

#elif defined(__linux__)
#include <fstream>
#include <string>

size_t getCurrentMemoryUsageKB() {
    std::ifstream status("/proc/self/status");
    std::string key;
    while (status >> key) {
        if (key == "VmRSS:") {
            size_t kb = 0;
            status >> kb;
            return kb;
        }
    }
    return 0;
}

#else
size_t getCurrentMemoryUsageKB() {
    return 0;
}
#endif
