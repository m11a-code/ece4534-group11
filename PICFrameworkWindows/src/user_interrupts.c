// This is where the "user" interrupts handlers should go
// The *must* be declared in "user_interrupts.h"

#include "maindefs.h"
#ifndef __XC8
#include <timers.h>
#else
#endif
#include "user_interrupts.h"
#include "messages.h"
#include <plib/timers.h>
#include "debug.h"

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    unsigned int val;
    int length, msgtype;

    // toggle an LED
    LATBbits.LATB0 = !LATBbits.LATB0;
    // reset the timer
    WriteTimer0(0);
    // try to receive a message and, if we get one, echo it back
    length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)(unsigned int)&msgtype, (void *) &val);
    if (length == sizeof (val)) {
        ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
    }
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;
    
    //Read the timer and tell main to read the A/D converter
    //Send the read timer value (not sure what to do with that yet)
    result = ReadTimer1();
    ToMainLow_sendmsg(sizeof(result), MSGT_TIMER1, (void *) &result);

    // reset the timer
    unsigned int temp = 0x1;
    WriteTimer1(temp);
}

void adc_int_handler() {
    unsigned int value = ReadADC();
    unsigned char message[3];
    message[0] = (unsigned char)(0xFF & value);
    message[1] = (unsigned char)(0xFF & (value>>8));
    message[2] = 0x10;
    ToMainLow_sendmsg(3,MSGT_I2C_DATA,(void *) message);
}
