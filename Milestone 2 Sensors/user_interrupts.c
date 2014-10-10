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
#include <plib/adc.h>
#include <stdlib.h>

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer0 interrupt

void timer0_int_handler() {
    //int length, msgtype;

    // toggle an LED
#ifdef __USE18F2680
    LATBbits.LATB0 = !LATBbits.LATB0;
#endif
    // reset the timer
    WriteTimer0(0);
    // try to receive a message and, if we get one, echo it back
    //length = FromMainHigh_recvmsg(sizeof(val), (unsigned char *)&msgtype, (void *) &val);
    //if (length == sizeof (val)) {
        //ToMainHigh_sendmsg(sizeof (val), MSGT_TIMER0, (void *) &val);
    //}

    //ADC_Start(0);
    unsigned char val = 0;
    //DEBUG_ON(TIMER0_MSG_SEND);
    ToMainHigh_sendmsg(sizeof(val), MSGT_TIMER0, (void*) &val);
    //DEBUG_OFF(TIMER0_MSG_SEND);
}

// A function called by the interrupt handler
// This one does the action I wanted for this program on a timer1 interrupt

void timer1_int_handler() {

    //Set debug pin high to show interrupt has occurred

    unsigned int result;

    // read the timer and then send an empty message to main()
#ifdef __USE18F2680
    LATBbits.LATB1 = !LATBbits.LATB1;
#endif

    result = ReadTimer1();
    ToMainLow_sendmsg(0, MSGT_TIMER1, (void *) 0);
    // reset the timer
    WriteTimer1(0);

    //Set debug pin back to low once interrupt handler is finished
}

void adc_int_handler()
{
    //DEBUG_ON(ADC_ISR);
    unsigned int val;
    //val = 0x041A;
    val = rand() % 1116 + 186;
    //val = ADRES;
    DEBUG_ON(ADC_MSG_SEND);
    ToMainHigh_sendmsg(sizeof(val), MSGT_SENSOR_DATA, (void*) &val);
    DEBUG_OFF(ADC_MSG_SEND);
    //DEBUG_OFF(ADC_ISR);
}