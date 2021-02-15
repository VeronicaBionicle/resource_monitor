#include "load_check.h"

static float CalculateCPULoad(ULL idleTicks, ULL totalTicks) {
	static ULL _previousTotalTicks = 0;
	static ULL _previousIdleTicks = 0;

	ULL totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
	ULL idleTicksSinceLastTime = idleTicks - _previousIdleTicks;

	float ret = 1.0f - ((totalTicksSinceLastTime > 0) ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

	_previousTotalTicks = totalTicks;
	_previousIdleTicks = idleTicks;

	return ret;
}

static ULL FileTimeToInt64(const FILETIME& ft) { return (((ULL)(ft.dwHighDateTime)) << 32) | ((ULL)ft.dwLowDateTime); }

// Returns 0...100% of CPU Load. -1 on error
double GetCPULoad() {
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? 100.0 * CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}

MEMORYSTATUSEX memoryStatusEx;

// Returns 0...100% of RAM Load
double GetRAMLoad() {
	memoryStatusEx.dwLength = sizeof(memoryStatusEx);	//init memory info
	GlobalMemoryStatusEx(&memoryStatusEx);
	return 100.0 * (1.0 - (double)memoryStatusEx.ullAvailPhys / memoryStatusEx.ullTotalPhys);
}
