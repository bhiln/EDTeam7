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
    DEBUG_ON(UART_RX);
    unsigned char error_msg;
    if (DataRdy1USART()) {
        DEBUG_ON(UART_TX);
//        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
//        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
//
        //uc_ptr->buflen++;
        // check if a message should be sent
//        if (uc_ptr->buflen == 5) {
//        if (uc_ptr->buflen == MAXUARTBUF) {
//            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
//            uc_ptr->buflen = 0;
//        }
        unsigned char length, forward[1], left[1], right[1], stop[1], reverse[1], readin[1];
        forward[0] = 0x0A;
        left[0] = 0x0B;
        right[0] = 0x0C;
        stop[0] = 0x0D;
        reverse[0] = 0x0E;
        readin[0] = Read1USART();

//        i2c_master_send(0x9A, 1, readin);
//        length = ToMainLow_sendmsg(1, MSGT_UART_DATA, readin);
        ToMainLow_sendmsg(1, MSGT_UART_DATA, readin);
        DEBUG_OFF(UART_TX);
        // check if a message should be sent
//        if (uc_ptr->buflen == 6 && uc_ptr->buffer[uc_ptr->buflen-1] == 0xFF) {
//        if (uc_ptr->buflen == 5) {
//
////            if (uc_ptr->buffer[0] == uc_ptr->cmd_count) {
////                if (uc_ptr->buffer[uc_ptr->buflen] == 0xFF) {
//                    ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
//                    uc_ptr->buflen = 0;
////                } else {
////                    // send an error
////                    // did not receive the stop byte 0xFF
////                    error_msg[0] = uc_ptr->cmd_count; // sends back the message index that the error occurred
////                    error_msg[1] = 0x1; // tells the ARM PIC that a stop byte was not received
////                    error_msg[2] = 0xFF;
////                    uart_send(3, error_msg); // tell the ARM PIC a stop byte was missed
////                    uc_ptr->cmd_count = uc_ptr->buffer[0];
////                }
////            } else {
////                // send an error
////                // missed a motor command
////                error_msg[0] = uc_ptr->cmd_count; // sends back the message index that the error occurred
////                error_msg[1] = 0x2; // tells the ARM PIC that a message was missed
////                error_msg[2] = 0xFF;
////                uart_send(3, error_msg); // tell the ARM PIC a message was missed
////                uc_ptr->cmd_count = uc_ptr->buffer[0];
////            }
//
//        }
//        ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
//        uc_ptr->cmd_count++;
//        ToMainLow_sendmsg(1, MSGT_UART_DATA, (void *) uc_ptr->buffer);
    }

    if (USART1_Status.OVERRUN_ERROR == 1) {
        // we've overrun the USART and must reset
        // send an error message for this
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        ToMainLow_sendmsg(0, MSGT_OVERRUN, (void *) 0);
    }
    DEBUG_OFF(UART_RX);
}

void uart_trans_int_handler() {
    DEBUG_ON(UART_TX);
    // error message
    if (uc_ptr->outbuflen == 3) {
        if (TXSTA1bits.TRMT == 1) {
            if (uc_ptr->outbufind < uc_ptr->outbuflen) {
                uc_ptr->outbufind++;
                TXREG1 = uc_ptr->outbuffer[uc_ptr->outbufind - 1];
            } else {
                uc_ptr->outbuflen = 0;
                PIE1bits.TX1IE = 0;
            }
        }
    } else {
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
    DEBUG_OFF(UART_TX);
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
    uc_ptr->cmd_count = 0;
}

void uart_send(unsigned char length, unsigned char *msg_buffer) {
    DEBUG_ON(UART_RX);
//    if (length == 23) {
        // sensor data
        uc_ptr->outbufind = 0;
        uc_ptr->outbuflen = length;
        msg_buffer[0] = uc_ptr->msg_count % 255;
        msg_buffer[1] = 0x8; // tells the ARM PIC that this is sensor data
        msg_buffer[22] = 0xFF;
        FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_I2C_DATA, (void *) msg_buffer);
        uc_ptr->msg_count++;
        PIE1bits.TXIE = 1;
        DEBUG_OFF(UART_RX);
//    } else if (length == 7) {
//        // motor data
//        uc_ptr->outbufind = 0;
//        uc_ptr->outbuflen = length;
//        msg_buffer[0] = uc_ptr->msg_count % 255;
//        msg_buffer[1] = 0x4; // tells the ARM PIC that this is quadrature encoder data
//        msg_buffer[6] = 0xFF;
//        FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_I2C_DATA, (void *) msg_buffer);
//        uc_ptr->msg_count++;
//        PIE1bits.TXIE = 1;
//        DEBUG_OFF(UART_RX);
//    }

//    // error message
//    // missed motor command
//    if (length == 3) {
//        uc_ptr->outbufind = 0;
//        uc_ptr->outbuflen = length;
//        uc_ptr->outbuffer = msg_buffer;
//        PIE1bits.TX1IE = 1;
//        DEBUG_OFF(UART_RX);
//    }
}
