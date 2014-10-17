#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"
#include "debug.h"

static uart_comm *uc_ptr;

void uart_recv_int_handler() {
    if (DataRdyUSART()) {
        uc_ptr->buffer[uc_ptr->buflen] = ReadUSART();

        uc_ptr->buflen++;
        // check if a message should be sent
        if (uc_ptr->buflen == MAXUARTBUF) {
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            uc_ptr->buflen = 0;
        }
        ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
    }

    if (USART_Status.OVERRUN_ERROR == 1) {
        // we've overrun the USART and must reset
        // send an error message for this
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        ToMainLow_sendmsg(0, MSGT_OVERRUN, (void *) 0);
    }
}

void uart_trans_int_handler() {
    FromMainLow_recvmsg(uc_ptr->outbuflen, MSGT_I2C_SLAVE_RECV_COMPLETE, (void *) uc_ptr->outbuffer);
    if (TXSTAbits.TRMT == 1) {
        if (uc_ptr->outbufind < uc_ptr->outbuflen) {
            uc_ptr->outbufind++;
            TXREG = uc_ptr->outbuffer[uc_ptr->outbufind - 1];
        } else {
            uc_ptr->outbuflen = 0;
            PIE1bits.TXIE = 0;
        }
    }
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
    uc_ptr->cmd_count = 0;
}

void uart_send(unsigned char length, unsigned char *msg_buffer) {
    uc_ptr->outbufind = 0;
    uc_ptr->outbuflen = length;
    FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_I2C_SLAVE_RECV_COMPLETE, (void *) msg_buffer);
    PIE1bits.TXIE = 1;
}