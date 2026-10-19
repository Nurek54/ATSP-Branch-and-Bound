#ifndef MEMORYUSAGE_H
#define MEMORYUSAGE_H

#include <cstddef>

// Pamiec zajmowana teraz przez caly proces, w KB
// (Windows: working set, Linux: VmRSS; na innych systemach 0)
size_t getCurrentMemoryUsageKB();

#endif // MEMORYUSAGE_H
