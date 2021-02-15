#include "COM_port.h"

HANDLE portHandle;
COMSTAT portState;
DWORD portErrors;
COMMTIMEOUTS portTimeouts;
DCB dcb;

int ScanAndChooseCOMPort() {
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	int r = 0;
	HKEY hkey = NULL;
	// Open Registry Key
	r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"HARDWARE\\DEVICEMAP\\SERIALCOMM\\", 0, KEY_READ, &hkey);
	if (r != ERROR_SUCCESS) return -1;

	unsigned long countOfPorts = 0, maxValueNameLen = 0, maxValueLen = 0;
	// Get data from registry
	RegQueryInfoKey(hkey, NULL, NULL, NULL, NULL, NULL, NULL, &countOfPorts, &maxValueNameLen, &maxValueLen, NULL, NULL);
	++maxValueNameLen;

	printf("Found %lu COM-ports:\n", countOfPorts);

	// Allocate memory for buffers
	CHAR* bufferName = NULL, * bufferData = NULL;

	bufferName = (CHAR*)malloc(maxValueNameLen * sizeof(TCHAR));
	if (!bufferName) {
		RegCloseKey(hkey);
		return -1;
	}

	bufferData = (CHAR*)malloc((maxValueLen + 1) * sizeof(TCHAR));
	if (!bufferData) {
		free(bufferName);
		RegCloseKey(hkey);
		return -1;
	}

	unsigned long nameLen, type, dataLen;
	// Print all found ports
	for (unsigned int i = 0; i < countOfPorts; i++)
	{
		nameLen = maxValueNameLen;
		dataLen = maxValueLen;
		r = RegEnumValue(hkey, i, (LPWSTR)bufferName, &nameLen, NULL, &type, (LPBYTE)bufferData, &dataLen);

		if ((r != ERROR_SUCCESS) || (type != REG_SZ)) continue;

		_tprintf(TEXT("%d) %s\n"), i + 1, bufferData);
	}

	printf("Enter port (0 to rescan): ");

	unsigned int choosenCOM;
	scanf_s("%u", &choosenCOM);

	if (choosenCOM <= 0 || choosenCOM > countOfPorts) {
		if (choosenCOM != 0) printf("Wrong port number\n");
		return -1;
	}

	nameLen = maxValueNameLen;
	dataLen = maxValueLen;
	r = RegEnumValue(hkey, choosenCOM - 1, (LPWSTR)bufferName, &nameLen, NULL, &type, (LPBYTE)bufferData, &dataLen);

	if ((r != ERROR_SUCCESS) || (type != REG_SZ)) {
		printf("Wrong port number\n");
		return -1;
	}

	int portNumber = 0;

	for (size_t i = 6; i < dataLen - 1; i++) {
		if (bufferData[i] >= '0' && bufferData[i] <= '9') { portNumber = portNumber * 10 + (bufferData[i] - '0'); };
	}

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	// Free memory
	free(bufferName);
	free(bufferData);
	// Close registry
	RegCloseKey(hkey);

	return portNumber;
}

bool OpenPort(LPCTSTR portName) {
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
		printf("Error setting serial port state\n");
		return false;
	}

	/* set timeouts
	*/
	portTimeouts.ReadIntervalTimeout = 10;
	portTimeouts.ReadTotalTimeoutMultiplier = 1;
	portTimeouts.ReadTotalTimeoutConstant = 100;
	portTimeouts.WriteTotalTimeoutMultiplier = 0;
	portTimeouts.WriteTotalTimeoutConstant = 0;
	SetCommTimeouts(portHandle, &portTimeouts);

	/* clear buffers
	*/
	PurgeComm(portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	return true;	// if everithing was ok, return true
}

bool StartPort() {
	int portNum = -1;
	TCHAR portName[32];
	while (portNum == -1) {
		system("cls");
		portNum = ScanAndChooseCOMPort();
	}
	_stprintf_s(portName, sizeof(portName) / sizeof(TCHAR), _T("\\\\.\\COM%d"), portNum);
	bool connectionStatus = OpenPort(portName);	// open port
	ShowWindow(GetConsoleWindow(), SW_HIDE);	//hide window
	return connectionStatus;
}

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

void ClosePort() {
	PurgeComm(portHandle, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	CloseHandle(portHandle);
};