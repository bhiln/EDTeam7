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
#include "debug.h"
#include "my_i2c.h"

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    unsigned int val;
    int length, msgtype;
    unsigned char i;
    
    if (i % 2 == 0) {
        // sensor
        i2c_master_recv(0x9E, 4);
    } // else if (i % 2 = 1) {
        // motor
//        i2c_master_recv(0x9A, 4);
//    }
    i++;

    // reset the timer
    WriteTimer0(0);
    // try to receive a message and, if we get one, echo it back
//    length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)&msgtype, (void *) &val);
//    if (length == sizeof (val)) {
//        ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
//    }
//
//    unsigned char i;
//    unsigned char forward[1], left[1], right[1], stop[1], reverse[1];
//    forward[0] = 0x0A;
//    left[0] = 0x0B;
//    right[0] = 0x0C;
//    stop[0] = 0x0D;
//    reverse[0] = 0x0E;
//
//    if (i % 5 == 0) {
//        ToMainHigh_sendmsg(1, MSGT_I2C_MOTOR_CMD, forward);
//        i = 0;
//    }
//    if (i % 5 == 1) {
//        ToMainHigh_sendmsg(1, MSGT_I2C_MOTOR_CMD, left);
//        i = 1;
//    }
//    if (i % 5 == 2) {
//        ToMainHigh_sendmsg(1, MSGT_I2C_MOTOR_CMD, right);
//        i = 2;
//    }
//    if (i % 5 == 3) {
//        ToMainHigh_sendmsg(1, MSGT_I2C_MOTOR_CMD, stop);
//        i = 3;
//    }
//    if (i % 5 == 4) {
//        ToMainHigh_sendmsg(1, MSGT_I2C_MOTOR_CMD, reverse);
//        i = 4;
//    }
//    i++;
//    ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;
//    unsigned char i;
//    unsigned char forward[1], left[1], right[1], stop[1], reverse[1];
//    forward[0] = 0x0A;
//    left[0] = 0x0B;
//    right[0] = 0x0C;
//    stop[0] = 0x0D;
//    reverse[0] = 0x0E;

    // read the timer and then send an empty message to main()
    result = ReadTimer1();
//    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);

//    if (i % 4 == 0) {
//        i2c_master_send(0x9A, 1, forward);
////        ToMainLow_sendmsg(1, MSGT_UART_DATA, forward);
//        i = 0;
////        i++;
//    } else if (i % 4 == 1) {
//        i2c_master_send(0x9A, 1, left);
////        ToMainLow_sendmsg(1, MSGT_UART_DATA, left);
//        i = 1;
////        i++;
//    } else if (i % 4 == 2) {
//        i2c_master_send(0x9A, 1, right);
////        ToMainLow_sendmsg(1, MSGT_UART_DATA, right);
//        i = 2;
////        i++;
//    }
////    else if (i % 5 == 3) {
////        ToMainLow_sendmsg(1, MSGT_UART_DATA, stop);
////        i = 3;
//////        i++;
////    }
//    else if (i % 4 == 3) {
//        i2c_master_send(0x9A, 1, reverse);
////        ToMainLow_sendmsg(1, MSGT_UART_DATA, reverse);
//        i = 3;
////        i++;
//    } // else if (i % 6 == 5) {
////        i = 0;
////        i++;
////    }
//    i++;
    // reset the timer
    WriteTimer1(0);
}