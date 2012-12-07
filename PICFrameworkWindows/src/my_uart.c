#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"
#ifndef __USE18F26J50

static uart_comm *uc_ptr, *uc_ptr2;

void uart_recv_int_handler() {
    if (DataRdyUSART()) {
        uc_ptr->buffer[uc_ptr->buflen] = ReadUSART();
        uc_ptr->buflen++;
        // check if a message should be sent
        if (uc_ptr->buflen == MAXUARTBUF) {
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            uc_ptr->buflen = 0;
        }
    }
    if (USART_Status.OVERRUN_ERROR == 1) {
        // we've overrun the USART and must reset
        // send an error message for this
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        ToMainLow_sendmsg(0, MSGT_OVERRUN, (void *) 0);
    }
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
}

void init_uart_send(uart_comm *uc) {
    uc_ptr2 = uc;
}

void uart_send(unsigned char length, unsigned char* msg){
    uc_ptr2->buflen = length;
    unsigned char indx = 0;
    while(indx < length){
        uc_ptr2->buffer[indx] = msg[indx];
        indx++;
    }
    WriteUSART(uc_ptr2->buffer[0]);
    uc_ptr2->buflen--;
    PIE1bits.TXIE = 1; //Enable TX interrupt
}

void uart_send_int_handler(){
    if(uc_ptr2->buflen > 0){
        WriteUSART(uc_ptr2->buffer[MAXUARTBUF - uc_ptr2->buflen]);
        uc_ptr2->buflen--;
    }else{
        PIE1bits.TXIE = 0; //Disable TX interrupt
    }
}
#endif