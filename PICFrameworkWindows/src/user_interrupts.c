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

static unsigned char adcMsgCount = 0;

static unsigned char leftEncoderCount = 0;

static unsigned char rightEncoderCount = 0;

#ifdef __MOTOR2680
static unsigned char prevRA0, prevRA1, prevRA2, prevRA3;
#endif

void init_encoder_counts(){
#ifdef __MOTOR2680
    leftEncoderCount = 0;
    rightEncoderCount = 0;
    prevRA0 = PORTAbits.RA0;
    prevRA1 = PORTAbits.RA1;
    prevRA2 = PORTAbits.RA2;
    prevRA3 = PORTAbits.RA3;
#endif
}

void encoder_int_handler(){
#ifdef __MOTOR2680
    if(prevRA0 == 0){
        if(PORTAbits.RA0 == 1){
            leftEncoderCount++;
        }
    }
    if(prevRA1 == 0){
        if(PORTAbits.RA1 == 1){
            leftEncoderCount++;
        }
    }
    if(prevRA2 == 0){
        if(PORTAbits.RA2 == 1){
            rightEncoderCount++;
        }
    }
    if(prevRA3 == 0){
        if(PORTAbits.RA3 == 1){
            rightEncoderCount++;
        }
    }
    prevRA0 = PORTAbits.RA0;
    prevRA1 = PORTAbits.RA1;
    prevRA2 = PORTAbits.RA2;
    prevRA3 = PORTAbits.RA3;
#endif
}

int get_right_encoder_count(){
    return rightEncoderCount;
}

int get_left_encoder_count(){
    return leftEncoderCount;
}

void timer0_int_handler() {
    // reset the timer
    WriteTimer0(0);
    unsigned int result = ReadTimer0();
    ToMainHigh_sendmsg(sizeof(result),MSGT_TIMER0,(void*) &result);
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    //Send the read timer value (not sure what to do with that yet)
    unsigned int result = ReadTimer1();
    // reset the timer
    WriteTimer1(0);
    ToMainLow_sendmsg(sizeof(result), MSGT_TIMER1, (void *) &result);
}

void adc_int_handler() {
#ifndef __SLAVE2680
    unsigned int value = ReadADC();
    value = 0x123;
    unsigned char message[I2C_MSG_SIZE];
    message[2] = (unsigned char)(0xFF & value); //Message Data
    message[3] = (unsigned char)(0xFF & (value>>8));
    //message[0] = ADC_MSG_TYPE;  //Message Type
    message[1] = adcMsgCount;   //Message Count
    adcMsgCount++;
    ToMainLow_sendmsg(I2C_MSG_SIZE,MSGT_I2C_DATA,(void *) message);
#endif
}
