#include "maindefs.h"
#include <stdio.h>
#include "messages.h"
#include "timer1_thread.h"
#include <plib/adc.h>
#include <p18cxxx.h>

void init_timer1_lthread(timer1_thread_struct *tptr) {
    //Initialize the ADC
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_0_TAD,
		ADC_CH0 & ADC_CH1 &
		ADC_INT_OFF & ADC_VREFPLUS_VDD &
		ADC_VREFMINUS_VSS, 0b1011);
    SetChanADC(ADC_CH0);
   // Delay10TCYx( 50 );
    ADC_INT_ENABLE(); //Enable Interrupts
    tptr->timerval = 0;
}

// This is a "logical" thread that processes messages from TIMER1
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int timer1_lthread(timer1_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    //Store the value of the timer into the struct
    tptr->timerval = msgbuffer[0];
    //Read value from ADC
    ConvertADC(); // Start conversion
}