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
#ifdef __USE18F26J50
    if (DataRdy1USART()) {
        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
#else
#ifdef __USE18F46J50
    if (DataRdy1USART()) {
        uc_ptr->buffer[uc_ptr->buflen] = Read1USART();
#else
    if (DataRdyUSART()) {
        uc_ptr->buffer[uc_ptr->buflen] = ReadUSART();
#endif
#endif

        uc_ptr->buflen++;
        // check if a message should be sent
        if (uc_ptr->buflen == MAXUARTBUF) {
            ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
            uc_ptr->buflen = 0;
        }
    }
#ifdef __USE18F26J50
    if (USART1_Status.OVERRUN_ERROR == 1) {
#else
#ifdef __USE18F46J50
    if (USART1_Status.OVERRUN_ERROR == 1) {
#else
    if (USART_Status.OVERRUN_ERROR == 1) {
#endif
#endif
        // we've overrun the USART and must reset
        // send an error message for this
        RCSTAbits.CREN = 0;
        RCSTAbits.CREN = 1;
        ToMainLow_sendmsg(0, MSGT_OVERRUN, (void *) 0);
    }
}

void uart_trans_int_handler() {
    DEBUG_ON(UART_TX);
    FromMainLow_recvmsg(uc_ptr->outbuflen, MSGT_UART_DATA, uc_ptr->outbuffer);
    if (TXSTA1bits.TRMT == 1) {
        if (uc_ptr->outbufind < uc_ptr->outbuflen) {
            uc_ptr->outbufind++;
            TXREG1 = uc_ptr->outbuffer[uc_ptr->outbufind-1];
        } else {
            uc_ptr->outbuflen = 0;
            PIE1bits.TX1IE = 0;
        }
    }
    DEBUG_OFF(UART_TX);
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
}

void uart_send(unsigned char length, unsigned char *msg_buffer) {
    DEBUG_ON(UART_RX);
    uc_ptr->outbufind = 0;
    uc_ptr->outbuflen = length;
    FromMainLow_sendmsg(uc_ptr->outbuflen, MSGT_UART_DATA, msg_buffer);
    PIE1bits.TX1IE = 1;
    DEBUG_OFF(UART_RX);
}
