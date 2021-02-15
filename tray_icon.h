#include <Windows.h>

#ifndef TRAY_ICON_H_
#define TRAY_ICON_H_

// Initializes icon
void InitIcon(HWND window);

// Change picture of icon (default if status = true, error, if false)
void SetIconPict(BOOL status);

// Update text in icons tip (notification)
void UpdateIconTip(wchar_t* tipText, int textLength);

// Delete tray icon
void DeleteIcon();

#endif  //TRAY_ICON_H_