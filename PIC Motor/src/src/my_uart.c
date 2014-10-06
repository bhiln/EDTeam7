#include "maindefs.h"
#ifndef __XC8
#include <usart.h>
#else
#include <plib/usart.h>
#endif
#include "my_uart.h"

static uart_comm *uc_ptr;
int index = 0;
//unsigned char i = 0x1;
unsigned char msgtest[6] = {0,0,0,0,0,0};

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

        ToMainLow_sendmsg(uc_ptr->buflen, MSGT_UART_DATA, (void *) uc_ptr->buffer);
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
    unsigned char msgbuffer[MSGLEN+1];
    unsigned char msgtype;
    signed char length;
    unsigned char start;
    length = SensorData_recvmsg(start, MSGLEN, &msgtype, (void *) msgbuffer);
    
    //unsigned char msg[6];
    //msg[0] = 0x0;
//    i = i << 1;
    if (msgtest[0] == 0xFF) {
        msgtest[0] = 0x0;
    }

    msgtest[0]++;
    msgtest[1] = 0xA;
    msgtest[2] = 0x00;
    msgtest[3] = msgtest[0];
    msgtest[4] = 0xFF;
    msgtest[5] = 0x0;

//    if (length == 0) {
//        PIE1bits.TX1IE = 0x0;
//    }
    TXREG1 = msgtest[index];

    //i++;
    index++;
    //FromMainHigh_recvmsg()
    if (index == 6) {
        index = 0;
        msgtest[0] = msgtest[0] - 0x5;
        PIE1bits.TX1IE = 0x0;
    }
}

void init_uart_recv(uart_comm *uc) {
    uc_ptr = uc;
    uc_ptr->buflen = 0;
}

void uart_send(int length, unsigned char *msg_buffer) {
    int i = 0;
    for (i; i < length; i++) {
        //TXREG1 = msg_buffer[i];
        uc_ptr->buffer[i] = msg_buffer[i];
    }
    PIE1bits.TX1IE = 0x0;
}