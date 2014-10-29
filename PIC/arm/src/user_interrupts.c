// This is where the "user" interrupts handlers should go
// The *must* be declared in "user_interrupts.h"

#include "maindefs.h"
#ifndef __XC8
#include <timers.h>
#else
#include <plib/timers.h>
#endif
#include "user_interrupts.h"
#include "messages.h"
#include "ADC.h"
#include "debug.h"

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    unsigned int val;
    int length, msgtype;

    // toggle an LED
#ifdef __USE18F2680
    LATBbits.LATB0 = !LATBbits.LATB0;
#endif

//    unsigned char i;
//    unsigned char forward[1], left[1], right[1];
//    forward[0] = 0x0A;
//    left[0] = 0x0B;
//    right[0] = 0x0C;
//
//    if (i % 3 == 0) {
//        ToMainHigh_sendmsg(1, MSGT_UART_CMD, forward);
////        uart_send(1, forward);
//        i = 0;
////        i++;
//    }
//    if (i % 3 == 1) {
//        ToMainHigh_sendmsg(1, MSGT_UART_CMD, left);
////        uart_send(1, left);
//        i = 1;
////        i++;
//    }
//    if (i % 3 == 2) {
//        ToMainHigh_sendmsg(1, MSGT_UART_CMD, right);
////        uart_send(1, right);
//        i = 2;
////        i++;
//    }
//    i++;

    // reset the timer
    WriteTimer0(0);
    // try to receive a message and, if we get one, echo it back
//    length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)&msgtype, (void *) &val);
//    if (length == sizeof (val)) {
//        ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
//    }
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
//    DEBUG_ON();
    unsigned int result;

    // read the timer and then send an empty message to main()
#ifdef __USE18F2680
    LATBbits.LATB1 = !LATBbits.LATB1;
#endif
    
    result = ReadTimer1();
//    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);

//    unsigned char i;
//    unsigned char forward[1], left[1], right[1];
//    forward[0] = 0x0A;
//    left[0] = 0x0B;
//    right[0] = 0x0C;
//
//    if (i % 3 == 0) {
//        ToMainLow_sendmsg(1, MSGT_UART_CMD, forward);
////        uart_send(1, forward);
//        i = 0;
////        i++;
//    }
//    if (i % 3 == 1) {
//        ToMainLow_sendmsg(1, MSGT_UART_CMD, left);
////        uart_send(1, left);
//        i = 1;
////        i++;
//    }
//    if (i % 3 == 2) {
//        ToMainLow_sendmsg(1, MSGT_UART_CMD, right);
////        uart_send(1, right);
//        i = 2;
////        i++;
//    }
//    i++;

    // reset the timer
    WriteTimer1(0);
}