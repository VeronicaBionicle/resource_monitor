#include "timer_1_class.h"

ISR(TIMER1_COMPA_vect)  //ISR for couners and Timer1
{
  timer1.isrCallback();
}

/*  Group of functions for interrupts*/
void Timer1::isrDefaultUnused() {} 
void (*Timer1::isrCallback)() = Timer1::isrDefaultUnused;

void Timer1::startTimerForInterrupt(uint32_t Period) {
  cli();
  period = Period;
  TIMSK1 = (1 << OCIE1A);
  TCCR1A = TCCR_A_FOR_TIMER_1;
  Timer1::setupTimer(Period);
  sei();
}

void Timer1::stopTimerCounter() {
  TCCR1B &= CS_STOP;
  TIMSK1 = 0;
  TCCR1A = 0;
  OCR1A = 0;
  period = 0;
}

/*Service function for calculating Timer1 registers*/
void Timer1::setupTimer(uint32_t Period){
   if (Period <= OCR_1_MAX / F_CPU_SH) {
    TCCR1B = CS_FOR_NO_PRESC_1;
    OCR1A = (Period * F_CPU_SH) - 1;
  } else {
    if (Period <= PRESCALER_1 * OCR_1_MAX / F_CPU_SH) {
      TCCR1B = CS_FOR_1_PRESC_1;
      OCR1A = (Period * F_CPU_SH) / PRESCALER_1 - 1;
    } else {
      if (Period <= PRESCALER_2 * OCR_1_MAX / F_CPU_SH) {
        TCCR1B = CS_FOR_2_PRESC_1;
        OCR1A = (Period * F_CPU_SH) / PRESCALER_2 - 1;
      } else {
        if (Period <= PRESCALER_3 / F_CPU_SH * OCR_1_MAX) {
          TCCR1B = CS_FOR_3_PRESC_1;
          OCR1A = (Period * F_CPU_SH) / PRESCALER_3 - 1;
        } else {
          if (Period <= PRESCALER_4 / F_CPU_SH * OCR_1_MAX) {
            TCCR1B = CS_FOR_4_PRESC_1;
            OCR1A = (Period * F_CPU_SH) / PRESCALER_4 - 1;
          } else {
            TCCR1B = CS_FOR_4_PRESC_1;
            OCR1A = OCR_1_MAX - 1;
          };
        };
      };
    };
  };
};
