#include <Windows.h>
#include <stdio.h>
//#include <tchar.h>

using namespace std;

#define TIMER_PERIOD 1000

#define CPU_LOAD_CHAR 'c'
#define RAM_LOAD_CHAR 'r'

typedef unsigned long long ULL;

NOTIFYICONDATA nid = {};

HANDLE hSerial;

void WriteCOM(char type_of_data, double load) {
	char data[7];
	DWORD dwSize = sizeof(data);
	DWORD dwBytesWritten;

	snprintf(data, dwSize, "%c%5.1f", type_of_data, load);

	BOOL iRet = WriteFile(hSerial, data, dwSize, &dwBytesWritten, NULL);

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
	return 100.0 * (1.0 - (double)memoryStatusEx.ullAvailPhys / memoryStatusEx.ullTotalPhys);
}

#define WM_TrayIcon (WM_APP + 1)
#define id_MyIcon 123

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	// Сообщение от значка
	case WM_USER:
		if (lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDBLCLK)
			if (MessageBox(NULL, TEXT("Завершить работу?"), TEXT("Resource monitor"), MB_ICONQUESTION | MB_YESNO) == IDYES)
				DestroyWindow(window);
		break;
		// Стандартная обработка сообщений
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}

VOID CALLBACK TimerFunc(HWND, UINT, UINT idTimer, DWORD dwTime) {
	double CPULoad = GetCPULoad();
	double RAMLoad = GetRAMLoad();

	WriteCOM(CPU_LOAD_CHAR, CPULoad);
	WriteCOM(RAM_LOAD_CHAR, RAMLoad);

	//update info in tip
	wchar_t tip_text [22];
	wsprintf(tip_text, L"CPU: %ld%% RAM: %ld%%", (long)CPULoad, (long)RAMLoad);
	lstrcpyn(nid.szTip, tip_text, 22);
	Shell_NotifyIcon(NIM_MODIFY, &nid); // ? S_OK : E_FAIL;
}

int main(HINSTANCE instance, HINSTANCE, LPTSTR, int)
{
	//init
	memoryStatusEx.dwLength = sizeof(memoryStatusEx);

	//COM-port choose \\.\COM%d
	LPCTSTR sPortName = L"\\\\.\\COM11";

	//"open" port
	hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	//if port was not created
	if (hSerial == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			printf("Serial port does not exist.\n");
		}
		printf("some other error occurred.\n");
	}

	if (!GetCommState(hSerial, &dcbSerialParams)) {
		printf("Getting state error\n");
	}

	//settings of port
	dcbSerialParams.BaudRate = CBR_9600;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	//add reconnect try?
	while (!SetCommState(hSerial, &dcbSerialParams)) {
		printf("error setting serial port state\n");
		hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		Sleep(500);
	};

	UINT timer = SetTimer(NULL, 0, TIMER_PERIOD, (TIMERPROC)TimerFunc);
	
	// Регистрация класса окна
	WNDCLASSEX main = { 0 };
	main.cbSize = sizeof(WNDCLASSEX);
	main.hInstance = instance;
	main.lpszClassName = TEXT("Main");
	main.lpfnWndProc = WndProc;
	RegisterClassEx(&main);
	
	// Создание главного окна
	HWND window = CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	nid.cbSize = sizeof(nid);
	nid.hWnd = window;
	nid.uVersion = NOTIFYICON_VERSION;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	nid.uCallbackMessage = WM_USER;
	// This text will be shown as the icon's tooltip.
	lstrcpyn(nid.szTip, L"CPU and RAM load", sizeof(nid.szTip) / sizeof(nid.szTip[0]));
	
	// Load the icon
	nid.hIcon = (HICON)LoadImage(NULL, TEXT("chip.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

	// add icon to tray
	bool icon_created = Shell_NotifyIcon(NIM_ADD, &nid); // ? S_OK : E_FAIL;

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	BOOL t = KillTimer(NULL, timer);
    // Удаление значка
	Shell_NotifyIcon(NIM_DELETE, &nid);
	CloseHandle(hSerial);

	return 0;
}
