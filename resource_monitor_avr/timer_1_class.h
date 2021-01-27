#ifndef timer_1_class_h_
#define timer_1_class_h_
#include <avr/interrupt.h>
#include <inttypes.h>

#define F_CPU_SH 16 //F_CPU/1000000 i had some problems with overflow
/*Units for startCounter function*/
#define MILLIS 1
#define SECONDS 975    //must be 1000
#define MINUTES 58500   //must be 60000, but my arduinos "minute" is 1 min + 1.5 sec
#define HOURS 3510000
              
/*Timer1 is 16-bit (period 1 us-4.19 s) */

class Timer1 {
  private:
    uint32_t period;
    void setupTimer(uint32_t Period);  //service function for calculating CS and OCR for timers
  public:
    void startTimerForInterrupt(uint32_t Period);  //start timer for attaching function to ISR, period`s unit measure is microseconds
    void stopTimerCounter();  //stop counting and interrupts from Timer
    static void (*isrCallback)();   //
    static void isrDefaultUnused(); //blank function, if function isn`t attached to ISR
    void attachTimerInterrupt(void (*isr)(), uint32_t Period) __attribute__((always_inline)) {  //attach function to ISR (function must be void without arguments!!!)
      startTimerForInterrupt(Period); //Period in microseconds
      isrCallback = isr;
    };
    void detachTimerInterrupt() __attribute__((always_inline)) {  //detach function from ISR and stop interrupts
      stopTimerCounter();
      isrCallback = isrDefaultUnused;
    };
};

/*  Some definitions for our timers */
  #define TCCR_A_FOR_TIMER_1 0
  #define OCR_1_MAX 65536 
  #define CS_FOR_COUNTER_1 (1 << WGM12)|(1 << CS10)
  #define OCR_FOR_COUNTER_1 0x3E7F  
  #define CS_STOP ~(1<<CS12)|(1<<CS11)|(0<<CS10)
  #define CS_FOR_NO_PRESC_1 (1 << WGM12) |(1 << CS10)
  #define PRESCALER_1 8
  #define CS_FOR_1_PRESC_1 (1 << WGM12) |(1 << CS11)
  #define PRESCALER_2 64
  #define CS_FOR_2_PRESC_1 (1 << WGM12) |(1 << CS11)|(1 << CS10)
  #define PRESCALER_3 256
  #define CS_FOR_3_PRESC_1 (1 << WGM12) |(1 << CS12)
  #define PRESCALER_4 1024
  #define CS_FOR_4_PRESC_1 (1 << WGM12) |(1 << CS12)|(1 << CS10)
  #define CS_FOR_COUNTER_1 CS_FOR_2_PRESC_1

extern Timer1 timer1;
#endif /*timer_1_class_h_ */
