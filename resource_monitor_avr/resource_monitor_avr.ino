#include <LiquidCrystal_I2C.h>
#include "timer_1_class.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define goToCPU lcd.setCursor(9,0)
#define goToRAM lcd.setCursor(9,1)

#define REGEN_TIME 1000000UL
#define REGEN_SECONDS 10

Timer1 regen_timer;

void regenerateDisplay() {
  lcd.clear();
  lcd.home();
  lcd.print("CPU Load:   0.0%");
  lcd.setCursor(0,1);
  lcd.print("RAM Load:   0.0%"); 
  }

void displayNoConnection() {
  lcd.clear();
  lcd.home();
  lcd.print("Waiting for");
  lcd.setCursor(0,1);
  lcd.print("connection...");
}

int ticks = 0;
int serial_not_connected = 0;

void tickCount() {
  ticks += serial_not_connected;
  }

void setup() {
  // initialize the lcd
  lcd.init();                      
  lcd.backlight();
  
  regen_timer.attachTimerInterrupt(tickCount, REGEN_TIME);
  
  displayNoConnection();
  
  Serial.begin(115200);
  while (Serial.available() == 0) {};
  
  regenerateDisplay();
}

char ch = 0;
String str = "";

void loop() {
  if (Serial.available() > 0) {  //если есть доступные данные
    serial_not_connected = 0;
    ticks = 0;
    if (ch = Serial.read()) {
      str += ch;
    }
   else {
    if (str[0] == 'c') {goToCPU;} else if (str[0] == 'r') {goToRAM;};
    str[0] = ' ';
    lcd.print(str);
    str = "";
    };
    
  } else {
    serial_not_connected = 1;
    }
    
  if (ticks >= REGEN_SECONDS && serial_not_connected) {
    displayNoConnection(); 
    while (Serial.available() == 0) {} ;
    serial_not_connected = 0;
    ticks = 0;
    regenerateDisplay();
    };
  /* write to com program
   * Serial.write(ch);
  Serial.write('\n');
  delay(100);
  if (++ch > 'Z') ch = 'A';*/
}
