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

static tmr0_comm *t0_ptr;

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    // untested
//    unsigned char done[1];
    // reset the timer
    WriteTimer0(200); // figure out how many ticks to go one inch
    unsigned char reply[1];
    
    t0_ptr->count++;
    if ((t0_ptr->count % t0_ptr->distance) == 0) {
        unsigned char length = 2;
        unsigned char msg[2];
        t0_ptr->count = 0;
//        reply[0] = 0xFE;
//        reply[1] = 0x35;
//        reply[2] = 0xFF;
        reply[0] = 0x35;
        MotorData_sendmsg(1, MSGT_I2C_RQST, (void *) reply);
        reply[0] = 0x0;
        msg[0] = 0x40;
        msg[1] = 0xC0;
        uart_send(length, msg);
    }
//    else {
//        reply[0] = 0x0;
//        MotorData_sendmsg(1, MSGT_I2C_RQST, (void *) reply);
//    }
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;

    // read the timer and then send an empty message to main()
    result = ReadTimer1();
//    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);

    // reset the timer
    WriteTimer1(0);
}

void init_tmr0(tmr0_comm *t0) {
    t0_ptr = t0;
    t0_ptr->count = 0;
    t0_ptr->distance = 0;
}