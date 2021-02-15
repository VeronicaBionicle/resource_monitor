#include "load_check.h"
#include "tray_icon.h"
#include "COM_port.h"

#define TIMER_PERIOD 1000

#define CPU_LOAD_CHAR 'c'
#define RAM_LOAD_CHAR 'r'

#define TIP_TEXT_LENGTH 22	//for string "CPU: %ld%% RAM: %ld%%"

bool connectionOk = false;

// callback for tray icon interaction
LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) { // Message from tray icon
	case WM_USER:
		if (lParam == WM_RBUTTONDOWN || lParam == WM_LBUTTONDBLCLK) {
			if (connectionOk) {
				int answer = MessageBox(NULL, TEXT("Завершить работу?"), TEXT("Resource monitor"), MB_ICONQUESTION | MB_YESNO);
				if (answer == IDYES) { DestroyWindow(window); } else break;
			} else {
					int answer = MessageBox(NULL, TEXT("Ошибка соединения. Переподключиться?"), TEXT("Resource monitor"), MB_ICONERROR | MB_ABORTRETRYIGNORE);
					switch (answer) {
					case IDABORT: PostQuitMessage(0); break;	//close
					case IDRETRY:
						ClosePort();
						connectionOk = StartPort();
						break;
					case IDIGNORE: ShowWindow(GetConsoleWindow(), SW_HIDE); break;
					default: break;
					}
			}
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
 
	connectionOk = WriteCOM(CPU_LOAD_CHAR, CPULoad);
	connectionOk &= WriteCOM(RAM_LOAD_CHAR, RAMLoad);
	
	SetIconPict(connectionOk);	//change picture of icon, if status of connection changed

	//update info in tip
	wchar_t tipText[TIP_TEXT_LENGTH];
	wsprintf(tipText, L"CPU: %ld%% RAM: %ld%%", (long)CPULoad, (long)RAMLoad);
	UpdateIconTip(tipText, TIP_TEXT_LENGTH);
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
	
	InitIcon(window);

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	UINT_PTR timer = SetTimer(NULL, 0, TIMER_PERIOD, (TIMERPROC)TimerFunc);	//start timer
	
	// message loop
	MSG message;
	while (GetMessage(&message, NULL, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	KillTimer(NULL, timer);	//stop timer
	DeleteIcon();
	ClosePort();

	return 0;
}