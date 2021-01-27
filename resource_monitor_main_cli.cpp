#include <Windows.h>
#include <stdio.h>

using namespace std;

#define TIMER_PERIOD 1000

#define CPU_LOAD_CHAR 'c'
#define RAM_LOAD_CHAR 'r'

typedef unsigned long long ULL;

HANDLE hSerial;

/*
void ReadCOM() {
	DWORD iSize;
	char sReceivedChar;
	while (true)
	{
		ReadFile(hSerial, &sReceivedChar, 1, &iSize, 0);
		if (iSize > 0) printf("%c", sReceivedChar);
	}
}
*/

void WriteCOM(char type_of_data, double load) {
	char data[7];
	DWORD dwSize = sizeof(data);
	DWORD dwBytesWritten;

	snprintf(data, dwSize, "%c%5.1f", type_of_data, load);

	BOOL iRet = WriteFile(hSerial, data, dwSize, &dwBytesWritten, NULL);
	//printf("Send %s (%d bytes). %d bytes sended!\n", data, dwSize, dwBytesWritten);

	if (!iRet) printf("Error! Send %s (%d bytes). %d bytes sended!\n", data, dwSize, dwBytesWritten);
}

MEMORYSTATUSEX memoryStatusEx;

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

// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
// You'll need to call this at regular intervals, since it measures the load between
// the previous call and the current one.  Returns -1.0 on error.
double GetCPULoad() {
	FILETIME idleTime, kernelTime, userTime;
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? 100.0*CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}

double GetRAMLoad() {
	GlobalMemoryStatusEx(&memoryStatusEx);
	//long RAMLoad = memoryStatusEx.dwMemoryLoad;
	return 100.0 * (1.0 - (double)memoryStatusEx.ullAvailPhys / memoryStatusEx.ullTotalPhys);
}

int main(){
	ShowWindow(GetConsoleWindow(), SW_HIDE);	//sw_minimize
	//init
	memoryStatusEx.dwLength = sizeof(memoryStatusEx);

	//COM-port choose
	LPCTSTR sPortName = L"\\\\.\\COM11";

	//"open" port
	hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	//if port was not created
	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND){
			printf("Serial port does not exist.\n");
		}
		printf("some other error occurred.\n");
	}

	if (!GetCommState(hSerial, &dcbSerialParams)){
		printf("Getting state error\n");
	}

	//settings of port
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (!SetCommState(hSerial, &dcbSerialParams)){
		printf("error setting serial port state\n");
	}

	//add reconnect try?

	//while (!_kbhit())   //until key was pressed. needs //#include <conio.h>
	while (1) 
	{
		double CPULoad = GetCPULoad();
		double RAMLoad = GetRAMLoad();

		//printf("CPU load    : %3.1f%%\n", CPULoad);
		//printf("Memory usage: %3.1f%%\n", RAMLoad);
		WriteCOM(CPU_LOAD_CHAR, CPULoad);
		WriteCOM(RAM_LOAD_CHAR, RAMLoad);
		Sleep(TIMER_PERIOD);
	}

	CloseHandle(hSerial);
	return 0;
}

