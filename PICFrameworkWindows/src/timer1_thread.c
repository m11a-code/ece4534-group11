#include "maindefs.h"
#include <stdio.h>
#include "messages.h"
#include "timer1_thread.h"
#include <plib/adc.h>
#include <p18cxxx.h>
#include "my_i2c.h"
#include "user_interrupts.h"

static unsigned char msgCount;
static unsigned char goFlag;

void init_timer1_lthread(timer1_thread_struct *tptr) {
    msgCount = 0;
    goFlag = 0;
#ifdef __USE18F45J10
    //Initialize the ADC
    OpenADC(ADC_FOSC_8 & ADC_RIGHT_JUST & ADC_0_TAD,
		ADC_CH0 & ADC_CH1 &
		ADC_INT_OFF & ADC_VREFPLUS_VDD &
		ADC_VREFMINUS_VSS, 0b1011);
    SetChanADC(ADC_CH0);
   // Delay10TCYx( 50 );
    ADC_INT_ENABLE(); //Enable Interrupts
#endif
    tptr->timerval = 0;
}

// This is a "logical" thread that processes messages from TIMER1
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int timer1_lthread(timer1_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    //Store the value of the timer into the struct
    tptr->timerval = msgbuffer[0];

#ifdef __MOTOR2680
    if(goFlag == 0){
        msgCount++;
        unsigned char msg[I2C_MSG_SIZE];
        msg[0] = ENCODERS_MSG_TYPE;
        msg[1] = msgCount;
        msg[2] = get_left_encoder_count();
        msg[3] = get_right_encoder_count();
        init_encoder_counts();
        FromMainLow_sendmsg(I2C_MSG_SIZE, MSGT_I2C_DATA, (void*) msg);
        goFlag = 1;
    }else{
        goFlag = 0;
    }
#endif
#ifdef __USE18F45J
    unsigned char toSend[] = {0xAB,0x55};
    i2c_master_send(2,toSend);
#else
    //Read value from ADC
#ifdef __USE18F45J10
    ConvertADC(); // Start conversion
#endif
#ifdef __MASTER2680
    unsigned char type;
    length = FromMainLow_recvmsg(MSGLEN, &type, (void*) msgbuffer);
    if(length > 0){
        switch(type){
            case MSGT_I2C_RQST:
            {
                //Change 0xAA if the sonar wants some sort of command
                i2c_master_recv(0xAA, 4);
                break;
            }
            case MSGT_UART_DATA:
            {
                i2c_configure_master(ENCODERS_ADDR);
                i2c_master_send(length, msgbuffer);
                break;
            }
            case MSGT_SONAR_SEND:
            {
                i2c_configure_master(SONAR_ADDR);
                unsigned char rangeCommand[I2C_MSG_SIZE];
                //rangeCommand[0] = 0xE0;
                rangeCommand[0] = 0x00;
                rangeCommand[1] = 0x51;
                i2c_master_send(2, rangeCommand);
                break;
            }
            case MSGT_SONAR_RECV:
            {
                i2c_configure_master(SONAR_ADDR);
                //unsigned char rangeCommand[I2C_MSG_SIZE];
                //rangeCommand[0] = 0x01;
                //2c_master_send(1, rangeCommand);
                i2c_master_recv(0x01,3);
                break;
            }
        }
    }else{
        // Do nothing
    }
#endif

#endif
}