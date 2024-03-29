#include "maindefs.h"
#include <stdio.h>
#ifndef __XC8
#include <usart.h>
#include <i2c.h>
#include <timers.h>
#else
#include <plib/usart.h>
#include <plib/i2c.h>
#include <plib/timers.h>
#endif
#include "interrupts.h"
#include "messages.h"
#include "my_uart.h"
#include "my_i2c.h"
#include "uart_thread.h"
#include "timer1_thread.h"
#include "timer0_thread.h"
#include "user_interrupts.h"

#ifdef __USE18F45J10
// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT disabled (control is placed on SWDTEN bit))
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable bit (Reset on stack overflow/underflow disabled)
#ifndef __XC8
// Have to turn this off because I don't see how to enable this in the checkboxes for XC8 in this IDE
#pragma config XINST = ON       // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)
#else
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)
#endif

// CONFIG1H
#pragma config CP0 = OFF        // Code Protection bit (Program memory is not code-protected)

// CONFIG2L
#pragma config FOSC = HSPLL     // Oscillator Selection bits (HS oscillator, PLL enabled and under software control)
#pragma config FOSC2 = ON       // Default/Reset System Clock Select bit (Clock selected by FOSC as system clock is enabled when OSCCON<1:0> = 00)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit (Two-Speed Start-up enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = DEFAULT // CCP2 MUX bit (CCP2 is multiplexed with RC1)

#else
#ifdef __USE18F2680
#pragma config OSC = IRCIO67    // Oscillator Selection bits (Internal oscillator block, port function on RA6 and RA7)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor disabled)
#pragma config IESO = OFF       // Internal/External Oscillator Switchover bit (Oscillator Switchover mode disabled)

// CONFIG2L
#pragma config PWRT = OFF       // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled in hardware and software)
#pragma config BORV = 3         // Brown-out Reset Voltage bits (VBOR set to 2.1V)

// CONFIG2H
#pragma config WDT = OFF        // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config PBADEN = OFF     // PORTB A/D Enable bit (PORTB<4:0> pins are configured as digital I/O on Reset)
#pragma config LPT1OSC = OFF    // Low-Power Timer 1 Oscillator Enable bit (Timer1 configured for higher power operation)
#pragma config MCLRE = ON       // MCLR Pin Enable bit (MCLR pin enabled; RE3 input pin disabled)

// CONFIG4L
#pragma config STVREN = OFF     // Stack Full/Underflow Reset Enable bit (Stack full/underflow will not cause Reset)
#pragma config LVP = OFF        // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config BBSIZ = 1024     // Boot Block Size Select bits (1K words (2K bytes) Boot Block)
#ifndef __XC8
// Have to turn this off because I don't see how to enable this in the checkboxes for XC8 in this IDE
#pragma config XINST = ON       // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)
#endif
#else
//Something is messed up
#endif
#endif //about:startpage --I commented this

void main(void) {
    char c;     //Is this even used?
    signed char length;
    unsigned char msgtype;
    unsigned char last_reg_recvd;
    uart_comm uc, uc2;
    i2c_comm ic;
    unsigned char msgbuffer[MSGLEN + 1];
    unsigned char sonarData[I2C_MSG_SIZE];
    unsigned char i;
    uart_thread_struct uthread_data; // info for uart_lthread

    //Timer values
    timer1_thread_struct t1thread_data; // info for timer1_lthread
    timer0_thread_struct t0thread_data; // info for timer0_lthread

#ifdef __USE18F2680
    OSCCON = 0xFC; // see datasheet
    // We have enough room below the Max Freq to enable the PLL for this chip
    OSCTUNEbits.PLLEN = 1; // 4x the clock speed in the previous line
#else
    OSCCON = 0x82; // see datasheeet
    OSCTUNEbits.PLLEN = 0; // Makes the clock exceed the PIC's rated speed if the PLL is on
#endif

    // initialize my uart recv handling code
    init_uart_recv(&uc);
    init_uart_send(&uc2);

    // initialize the i2c code
    init_i2c(&ic);

#ifndef __SLAVE2680
    // init the timer1 lthread
    init_timer1_lthread(&t1thread_data);
#ifdef __MASTER2680
    init_timer0_lthread(&t0thread_data);
#endif
#ifdef __MOTOR2680
    init_timer0_lthread(&t0thread_data);
#endif
#endif

    // initialize message queues before enabling any interrupts
    init_queues();

    // set direction for PORTB to output
    TRISB = 0x0;
    LATB = 0x0;

    // how to set up PORTA for input (for the V4 board with the PIC2680)
    
            PORTA = 0x0;	// clear the port
            LATA = 0x0;		// clear the output latch
            ADCON1 = 0x0F;	// turn off the A2D function on these pins
            // Only for 40-pin version of this chip CMCON = 0x07;	// turn the comparator off
            TRISA = 0x0F;	// set RA3-RA0 to inputs

    // initialize Timers
    //OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_128);
#ifdef __USE18F2680
#ifdef __MOTOR2680
    OpenTimer0(TIMER_INT_ON & T0_PS_1_8 & T0_8BIT & T0_SOURCE_INT);
    OpenTimer1(TIMER_INT_ON & T1_PS_1_8 & T1_16BIT_RW & T1_SOURCE_INT & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF);
#endif
#ifdef __MASTER2680
    OpenTimer0(TIMER_INT_ON & T0_PS_1_2 & T0_16BIT & T0_SOURCE_INT);
    OpenTimer1(TIMER_INT_ON & T1_PS_1_2 & T1_16BIT_RW & T1_SOURCE_INT & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF);
#endif
#else
#ifdef __USE18F45J10
    OpenTimer1(TIMER_INT_ON & T1_PS_1_8 & T1_16BIT_RW & T1_SOURCE_INT & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF);
#else
    OpenTimer1(TIMER_INT_ON & T1_PS_1_8 & T1_16BIT_RW & 0xFD & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF,0x1);
#endif
#endif

    // Peripheral interrupts can have their priority set to high or low
    // enable high-priority interrupts and low-priority interrupts
    enable_interrupts();

    IPR1bits.TMR1IP = 0;
    // ADC interrupt
    IPR1bits.ADIP = 0;
    // USART RX interrupt
    IPR1bits.RCIP = 0;
    // I2C interrupt
    IPR1bits.SSPIP = 1;

#ifdef __SLAVE2680
    i2c_configure_slave(0x9E);
#endif

#ifdef __MOTOR2680
    i2c_configure_slave(ENCODERS_ADDR);
    init_encoder_counts();
#endif

#ifdef __USE18F45J10
    i2c_configure_slave(CAMERA_ADDR);
#endif

    PIE1bits.SSPIE = 1; //Enable interrupt for the MSSP (I2C module)

#ifdef __MOTOR2680
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
            USART_CONT_RX & USART_BRGH_LOW, 0x1A);
#endif
#ifdef __SLAVE2680
    // configure the hardware USART device
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
            USART_CONT_RX & USART_BRGH_LOW, 0x33);
#endif
#ifdef __MASTER2680
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
            USART_CONT_RX & USART_BRGH_LOW, 0x33);
#endif

    // loop forever
    // This loop is responsible for "handing off" messages to the subroutines
    // that should get them.  Although the subroutines are not threads, but
    // they can be equated with the tasks in your task diagram if you
    // structure them properly.
#ifdef __MASTER2680
    unsigned char sonarMsgCount = 0;
#endif
    while (1) {
        // Call a routine that blocks until either on the incoming
        // messages queues has a message (this may put the processor into
        // an idle mode)
        block_on_To_msgqueues();

        // At this point, one or both of the queues has a message.  It
        // makes sense to check the high-priority messages first -- in fact,
        // you may only want to check the low-priority messages when there
        // is not a high priority message.  That is a design decision and
        // I haven't done it here.
        length = ToMainHigh_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message, check the error code to see if it is concern
            if (length != MSGQUEUE_EMPTY) {
                // This case be handled by your code.
            }
        } else {
            switch (msgtype) {
                case MSGT_I2C_MASTER_SEND_COMPLETE:
                {
                    break;
                }
                case MSGT_I2C_MASTER_SEND_FAILED:
                {
                    //handle error
                    break;
                }
                case MSGT_I2C_MASTER_RECV_COMPLETE:
                {
#ifdef __MASTER2680
                    if(length == 3) {
                        sonarMsgCount++;
                        sonarData[0] = SONAR_MSG_TYPE;
                        sonarData[1] = sonarMsgCount;
                        sonarData[2] = msgbuffer[1];
                        sonarData[3] = msgbuffer[2];
                        uart_lthread(&uthread_data, MSGT_UART_DATA, 4, sonarData);
                    }
                    else{
                        uart_lthread(&uthread_data, MSGT_UART_DATA, length, msgbuffer);
                    }

#endif
                    break;
                }
                case MSGT_TIMER0:
                {
                    timer0_lthread(&t0thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_I2C_DATA:

                case MSGT_I2C_DBG:
                {
                    // Here is where you could handle debugging, if you wanted
                    // keep track of the first byte received for later use (if desired)
                    last_reg_recvd = msgbuffer[0];
                    break;
                };
                case MSGT_I2C_RQST:
                {
                    break;
                };
                default:
                {
                    // Your code should handle this error
                    break;
                };
            };
        }

        // Check the low priority queue
        length = ToMainLow_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message, check the error code to see if it is concern
            if (length != MSGQUEUE_EMPTY) {
                // Your code should handle this situation
            }
        } else {
            switch (msgtype) {
                case MSGT_TIMER1:
                {
                    timer1_lthread(&t1thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_I2C_DATA:
                {
                    //Send value over I2C
                    FromMainLow_sendmsg(length, msgtype, (void *) msgbuffer);
                };
                case MSGT_OVERRUN:
                case MSGT_UART_DATA:
                {
#ifdef __SLAVE2680
                    uart_lthread(&uthread_data, msgtype, length, msgbuffer);
#endif
#ifdef __MASTER2680
                    FromMainLow_sendmsg(length, msgtype, (void*) msgbuffer);
#endif
                    break;
                };
                default:
                {
                    // Your code should handle this error
                    break;
                };
            };
        }
    }

}