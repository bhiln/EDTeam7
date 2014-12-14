#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"
#include "debug.h"
#include "my_i2c.h"

static uart_comm *uc_ptr;

void uart_recv_int_handler() {
    if (DataRdy1USART()) {
        unsigned char readin[7], msgCountEcho[4];
        readin[uc_ptr->cmd_count] = Read1USART();

        if (readin[uc_ptr->cmd_count] == 0xFE) {
            uc_ptr->cmd_count = 0;
        } else if (readin[uc_ptr->cmd_count] == 0xFF) {
            ToMainLow_sendmsg(uc_ptr->cmd_count, MSGT_UART_DATA, readin);
//            ToMainHigh_sendmsg(uc_ptr->cmd_count, MSGT_I2C_MOTOR_CMD, readin);
            msgCountEcho[0] = 0xFE;
            msgCountEcho[1] = 0x33;
            msgCountEcho[2] = readin[0];
            msgCountEcho[3] = 0xFF;
            uart_send(4, msgCountEcho);

            uc_ptr->cmd_count = 0;
        } else {
            uc_ptr->cmd_count++;
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
    uc_ptr->cmd_count = 0;
}

void uart_send(unsigned char length, unsigned char *msg_buffer) {
    uc_ptr->outbufind = 0;
    uc_ptr->outbuflen = length;

    FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_I2C_DATA, (void *) msg_buffer);
    PIE1bits.TXIE = 1;
}

void uart_trans_int_handler() {
    FromMainLow_recvmsg(uc_ptr->outbuflen, (void *) MSGT_I2C_DATA, (void *) uc_ptr->outbuffer);
    if (TXSTA1bits.TRMT == 1) {
        if (uc_ptr->outbufind < uc_ptr->outbuflen) {
            uc_ptr->outbufind++;
            TXREG1 = uc_ptr->outbuffer[uc_ptr->outbufind - 1];
        } else {
            uc_ptr->outbuflen = 0;
            PIE1bits.TX1IE = 0;
        }
    }
}