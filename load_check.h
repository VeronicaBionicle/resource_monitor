#include <Windows.h>

#ifndef LOAD_CHECK_H_
#define LOAD_CHECK_H_

typedef unsigned long long ULL;

static float CalculateCPULoad(ULL idleTicks, ULL totalTicks);

static ULL FileTimeToInt64(const FILETIME& ft);

// Returns 0...100% of CPU Load. -1 on error
double GetCPULoad();

// Returns 0...100% of RAM Load
double GetRAMLoad();

#endif  // LOAD_CHECK_H_
