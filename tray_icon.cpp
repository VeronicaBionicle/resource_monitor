#include "tray_icon.h"

NOTIFYICONDATA trayIcon = {};
HICON defaultIcon = (HICON)LoadImage(NULL, TEXT("chip.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
HICON errorIcon = (HICON)LoadImage(NULL, TEXT("error.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

void InitIcon(HWND window) {
	// init tray icon
	trayIcon.cbSize = sizeof(trayIcon);
	trayIcon.hWnd = window;
	trayIcon.uVersion = NOTIFYICON_VERSION;
	trayIcon.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP;
	trayIcon.uCallbackMessage = WM_USER;
	// load the icon picture
	trayIcon.hIcon = defaultIcon;
	// add icon to tray
	Shell_NotifyIcon(NIM_ADD, &trayIcon);
}

void SetIconPict(BOOL status) {
	if (!status) {
		trayIcon.hIcon = errorIcon; // set error icon
	} else {
		trayIcon.hIcon = defaultIcon; //set normal icon
	}
}

void DeleteIcon() {
	Shell_NotifyIcon(NIM_DELETE, &trayIcon);
}

void UpdateIconTip(wchar_t * tipText, int textLength) {
	if (lstrcpyn(trayIcon.szTip, tipText, textLength))
		Shell_NotifyIcon(NIM_MODIFY, &trayIcon);
}