#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"

static uart_comm *uc_ptr;

void uart_recv_int_handler() {
    unsigned char echo[3];
    unsigned char readin[23];
    if (DataRdy1USART()) {
        readin[uc_ptr->msg_count] = Read1USART();

        // check if a message should be sent
//        if (readin[5] == 0xFF) {
        if(readin[uc_ptr->msg_count] == 0xFF)
        {
//            readin[0] = 0x1;
//            ToMainLow_sendmsg(uc_ptr->msg_count, MSGT_UART_DATA, (void *) readin);
//            ToMainLow_sendmsg(6, MSGT_UART_DATA, test);
            SensorData_sendmsg(23, MSGT_I2C_RQST, readin);

            //SensorData_sendmsg(22, MSGT_I2C_RQST, test);
//            echo[0] = 0x33;
//            echo[1] = readin[0];
//            echo[2] = 0xFF;
//            uart_send(3, echo);
            uc_ptr->msg_count = 0;
        }
        else {
            uc_ptr->msg_count++;
        }
    }

    if (USART1_Status.OVERRUN_ERROR == 1) {
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
    uc_ptr->bufind = 0;
    uc_ptr->outbuflen = 0;
    uc_ptr->outbufind = 0;
    uc_ptr->msg_count = 0;
}

void uart_trans_int_handler() {
    FromMainLow_recvmsg(uc_ptr->outbuflen, (void *) MSGT_I2C_DATA, (void *) uc_ptr->outbuffer);
    if (TXSTAbits.TRMT == 1) {
        uc_ptr->outbufind++;
        if (uc_ptr->outbufind < uc_ptr->outbuflen) {
            TXREG = uc_ptr->outbuffer[uc_ptr->outbufind - 1];
        } else {
            uc_ptr->outbuflen = 0;
            uc_ptr->outbufind = 0;
            PIE1bits.TXIE = 0;
        }
    }
}

void uart_send(int length, unsigned char *msg_buffer) {
    uc_ptr->outbufind = 0;
    uc_ptr->outbuflen = length;
    FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_I2C_DATA, (void *) msg_buffer);
    PIE1bits.TXIE = 1;
}