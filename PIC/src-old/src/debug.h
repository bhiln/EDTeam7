/* 
 * File:   debug.h
 * Author: Leah
 *
 * Created on September 18, 2014, 8:38 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H

#define DO_DEBUG

#define TIMER0_ISR LATBbits.LATB0
#define ADC_READ LATBbits.LATB1
#define TIMER0_MSG_SEND LATBbits.LATB2
#define TIMER0_MSG_RCV LATBbits.LATB3


#ifdef DO_DEBUG
#define DEBUG_ON(a) {\
    a = 1;\
}

#define DEBUG_OFF(a) {\
    a = 0;\
}
#else
#define DEBUG_ON(a)
#define DEBUG_OFF(a)

#endif

#endif	/* DEBUG_H */

