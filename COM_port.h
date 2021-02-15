#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#ifndef COM_PORT_H_
#define COM_PORT_H_

#define PORT_BUFFER_SZ 64

/*	Get list of ports from registry, prints out
	Then asks to choose port to connect	*/
int ScanAndChooseCOMPort();

/* Creates port file and inits it */
bool OpenPort(LPCTSTR portName);

/* Scan, ask user number and open port until connection is ok*/
bool StartPort();

/*	write char data about load to COM-port
	typeOfData: letter 'c' or 'r' (for CPU and RAM)
	load: 0.0-100.0
*/
bool WriteCOM(char typeOfData, double load);

// Clear buffer TX/RX and close connection 
void ClosePort();

#endif  //COM_PORT_H_