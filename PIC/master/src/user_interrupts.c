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
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {
    unsigned int result;

    // read the timer and then send an empty message to main()
    result = ReadTimer1();

    // reset the timer
    WriteTimer1(0);
}