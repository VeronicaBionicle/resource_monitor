# Resource monitor v.0.1
This repo contains Windows app and Arduino/AVR firmware for my "hardware" resource monitor.
Windows app uses WinAPI and : 
1. takes info about RAM load with sysinfoapi.h and calculates CPU load
2. sends it via COM-port to MCU
3. shows in CLI/tray
Arduino/AVR MCU:
1. takes info from COM-port
2. puts it on 16x2 LCD

### v.0.1 (27.01.2021) features
* RAM and CPU load info is as accurate as Windows Task Manager
* Windows app has CLI (uses less RAM) and TRAY (looks C00Ler) versions  
> TRAY version puts the icon to taskbar and hides CLI
>> Hover over icon to watch current CPU and RAM load
>> Double click / Right mouse button calls message box, which offers to quit app.  
> CLI version shows all info itself
* App try to reconnect on initialization, if MCU wasn`t found
* MCU checks connection every 10s and writes messages and try to reconnect, if it can`t find PC

### TO DO in v.0.2
* Clean up structure of Windows app:
1. Open window and create icon
2. Start showing loads in icon (unnessary)
3. Ask COM-port number
4. Try to open connection - write error messages and reconnect
5. Hide window after opening connection
6. Check connection with MCU with Timer
* Try to understand, what to do with .ico in .exe (in Debug ver it`s invisible)
* Make .h and .cpp files for functions
* Test, test, test!
* Start making MCU firmware on C/C++, not Wiring

### TO DO in v.1.0
* Draw own icon
* Make "normal" MCU firmware
* Compile Release exe
