# Resource monitor v.0.1.4
This repo contains Windows app and Arduino/AVR firmware for my "hardware" resource monitor.

Windows app uses WinAPI and : 
1. takes info about RAM load with sysinfoapi.h and calculates CPU load
2. sends it via COM-port to MCU
3. shows in CLI/tray

Arduino/AVR MCU:
1. takes info from COM-port
2. puts it on 16x

### NEW in v.0.1.4 (06.03.2021)
* New icons drawn by me
* Icons are added to resources (icons.rc)
* Compiled Release version

### NEW in v.0.1.3 (15.02.2021)
* Made .h and .cpp files for functions
* Cleared some names

### NEW in v.0.1.2 (06.02.2021)
* Added COM port scanning and choice
* Added reconnect dialog box
* Now reconnect works much better

### NEW in v.0.1.1 (28.01.2021)
* Added try to reconnect (works only on first plug...)
* Icon tip shows load without connections
* Added Error icon - it shows that connection with MCU is lost
* Cleared structure of app a little
* Cleared names of variables and functions

### v.0.1 (27.01.2021) features
* RAM and CPU load info is as accurate as Windows Task Manager
* Windows app has CLI (uses less RAM) and TRAY (looks C00Ler) versions  
> TRAY version puts the icon to taskbar and hides CLI
>> - Hover over icon to watch current CPU and RAM load
>> - Double click / Right mouse button calls message box, which offers to quit app, or reconnect device.  
>
> CLI version shows all info itself
* App try to reconnect on initialization, if MCU wasn`t found
* MCU checks connection every 10s and writes messages and try to reconnect, if it can`t find PC

### TO DO in v.0.2
* Make popup menu for tray icon instead of windows
* Check connection with PC with Timer
* Test, test, test!
* Start making MCU firmware on C/C++, not Wiring
* Make "normal" MCU firmware