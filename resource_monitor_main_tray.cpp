#include <Windows.h>
#include <stdio.h>

typedef unsigned long long ULL;

using namespace std;

#define TIMER_PERIOD 1000

#define CPU_LOAD_CHAR 'c'
#define RAM_LOAD_CHAR 'r'

#define PORT_BUFFER_SZ 64

NOTIFYICONDATA trayIcon = {};
HICON defaultIcon = (HICON)LoadImage(NULL, TEXT("chip.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
HICON errorIcon = (HICON)LoadImage(NULL, TEXT("error.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

MEMORYSTATUSEX memoryStatusEx;

HANDLE portHandle;
COMSTAT portState;
LPCTSTR portName = L"\\\\.\\COM11";
DWORD portErrors;
COMMTIMEOUTS portTimeouts;
DCB dcb;

/* Creates port file and inits it */
bool OpenPort (LPCTSTR portName) {
	portHandle = ::CreateFile(portName, GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (portHandle == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			wprintf(L"Serial port %s does not exist.\n", portName);
		}
		else printf("COM-port handle error.\n");
		return false;
	}

	SetupComm(portHandle, PORT_BUFFER_SZ, PORT_BUFFER_SZ);

	if (!GetCommState(portHandle, &dcb)) {
		printf("Getting state error\n");
		return false;
	}

	/* init port */
	dcb.BaudRate = CBR_115200;
	dcb.fBinary = TRUE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = 1;

	if (!SetCommState(portHandle, &dcb)) {
		printf("error setting serial port state\n");
		return false;
	}

	/* set timeouts */
	portTimeouts.ReadIntervalTimeout = 10;
	portTimeouts.ReadTotalTimeoutMultiplier = 1;
	portTimeouts.ReadTotalTimeoutConstant = 100;
	portTimeouts.WriteTotalTimeoutMultiplier = 0;
	portTimeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(portHandle, &portTimeouts);

	/* clear buffers */
	PurgeComm(portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	return true;	// if everithing was ok, return true
}

/*	write char data about load to COM-port
	data format: letter 'c' or 'r' (for CPU and RAM) and 000.0% float
*/
bool WriteCOM(char typeOfData, double load) {
	char data[7];
	DWORD dwBytesWritten;
	
	// format string
	snprintf(data, sizeof(data), "%c%5.1f", typeOfData, load);

	//clear and take errors
	ClearCommError(portHandle, &portErrors, &portState);

	BOOL iRet = false;
	if (!portErrors) {
		iRet = WriteFile(portHandle, data, sizeof(data), &dwBytesWritten, NULL);
	}
	return iRet;
}

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
	return GetSystemTimes(&idleTime, &kernelTime, &userTime) ? 100.0*CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0f;
}

// Returns 0...100% of RAM Load
double GetRAMLoad() {
	GlobalMemoryStatusEx(&memoryStatusEx);
	return 100.0 * (1.0 - (double)memoryStatusEx.ullAvailPhys / memoryStatusEx.ullTotalPhys);
}

// callback for tray icon interaction
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) { // Message from tray icon
	case WM_USER:
		if (lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDBLCLK) {
			if (MessageBox(NULL, TEXT("Завершить работу?"), TEXT("Resource monitor"), MB_ICONQUESTION | MB_YESNO) == IDYES)
				DestroyWindow(window);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(window, message, wParam, lParam);
	}
	return 0;
}

// callback for timer
// update load data, put it in icon tip and COM-port
VOID CALLBACK TimerFunc(HWND, UINT, UINT idTimer, DWORD dwTime) {
	double CPULoad = GetCPULoad();
	double RAMLoad = GetRAMLoad();

	bool writeOk = WriteCOM(CPU_LOAD_CHAR, CPULoad);
	writeOk &= WriteCOM(RAM_LOAD_CHAR, RAMLoad);

	if (!writeOk) {
		trayIcon.hIcon = errorIcon; // set error icon
		OpenPort(portName); //try to reconnect
		//ShowWindow(GetConsoleWindow(), SW_SHOW);	//for debugging
	} else {
		trayIcon.hIcon = defaultIcon; //set normal icon
	}
	
	//update info in tip
	wchar_t tipText[22];
	wsprintf(tipText, L"CPU: %ld%% RAM: %ld%%", (long)CPULoad, (long)RAMLoad);
	if (lstrcpyn(trayIcon.szTip, tipText, 22))
		Shell_NotifyIcon(NIM_MODIFY, &trayIcon);
}

int main(HINSTANCE instance, HINSTANCE, LPTSTR, int) {
	// init window
	// register class of main window
	WNDCLASSEX main = { 0 };
	main.cbSize = sizeof(WNDCLASSEX);
	main.hInstance = instance;
	main.lpszClassName = TEXT("Main");
	main.lpfnWndProc = WndProc;
	RegisterClassEx(&main);
	// create main window
	HWND window = CreateWindowEx(0, TEXT("Main"), NULL, 0, 0, 0, 0, 0, NULL, NULL, instance, NULL);

	// init tray icon
	trayIcon.cbSize = sizeof(trayIcon);
	trayIcon.hWnd = window;
	trayIcon.uVersion = NOTIFYICON_VERSION;
	trayIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	trayIcon.uCallbackMessage = WM_USER;
	// load the icon
	trayIcon.hIcon = defaultIcon;
	// add icon to tray
	Shell_NotifyIcon(NIM_ADD, &trayIcon);

	memoryStatusEx.dwLength = sizeof(memoryStatusEx);	//init memry info 

	OpenPort(portName);	// open port

	ShowWindow(GetConsoleWindow(), SW_HIDE);	//hide window
 
	UINT timer = SetTimer(NULL, 0, TIMER_PERIOD, (TIMERPROC)TimerFunc);	//start timer
	
	// message loop
	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	//stop timer
	KillTimer(NULL, timer);
    // delete tray icon
	Shell_NotifyIcon(NIM_DELETE, &trayIcon);
	// clear buffers and close port
	PurgeComm(portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	CloseHandle(portHandle);

	return 0;
}
