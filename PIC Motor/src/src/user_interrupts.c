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

    //Set debug pin high to show interrupt has occurred
    DEBUG_ON(TIMER0_ISR);

    unsigned int val;
    int length, msgtype;

    // toggle an LED
#ifdef __USE18F2680
    LATBbits.LATB0 = !LATBbits.LATB0;
#endif

    //PIE1bits.TX1IE = 0x1;
    uart_trans_int_handler();
    // reset the timer
    WriteTimer0(0);
    // try to receive a message and, if we get one, echo it back
    length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)&msgtype, (void *) &val);
    if (length == sizeof (val)) {
        ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
    }

    val = ADC_Read(0);
    DEBUG_ON(TIMER0_MSG_SEND);
    ToMainHigh_sendmsg(sizeof(val), MSGT_TIMER0, (void*) &val);
    DEBUG_OFF(TIMER0_MSG_SEND);

    //i2c_master_recv(4, 0x9E);
    //SSP1CON2bits.SEN = 1;
    //Set debug pin back to low once interrupt handler is finished
    DEBUG_OFF(TIMER0_ISR);
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {

    //Set debug pin high to show interrupt has occurred
    DEBUG_ON(TIMER1_ISR);

    unsigned int result;

    // read the timer and then send an empty message to main()
#ifdef __USE18F2680
    LATBbits.LATB1 = !LATBbits.LATB1;
#endif
    unsigned char imsg[4];
    imsg[0] = 0x11;
    imsg[1] = 0x22;
    imsg[2] = 0x33;
    imsg[3] = 0x44;
    
    i2c_master_send(4, imsg, 0x9E);
    //i2c_master_recv(4, 0x9E);
    result = ReadTimer1();
    DEBUG_ON(TIMER1_MSG_SEND);
    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);
    DEBUG_OFF(TIMER1_MSG_SEND);
    // reset the timer
    WriteTimer1(0);

    //Set debug pin back to low once interrupt handler is finished
    DEBUG_OFF(TIMER1_ISR);
}