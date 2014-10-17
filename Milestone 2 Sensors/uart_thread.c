#include "maindefs.h"
#include <stdio.h>
#include "uart_thread.h"

// This is a "logical" thread that processes messages from the UART
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int uart_lthread(uart_thread_struct *uptr, int msgtype, int length, unsigned char *msgbuffer) {
    if (msgtype == MSGT_OVERRUN) {
    }
    else if (msgtype == MSGT_UART_DATA) {
        // print the message (this assumes that the message
        // 		was a printable string)

        unsigned int *msgval;
        msgval = (unsigned int *) msgbuffer;
        unsigned int val = *msgval;

        unsigned char temp = 0;

        msgbuffer[length] = '\0'; // null-terminate the array as a string
        // Now we would do something with it
        //TXREG1 = (0 | val);
        //TXREG1 = (0 | (val >> 8));
        //TXREG1 = msgbuffer[0];
        //TXREG1 = msgbuffer[1];
        //TXREG1 = 'a';
        //TXREG1 = 0x6F;
        TXREG1 = 'o';
        TXREG1 = 0x3;

    }
}