/*
 * File:   debug.h
 * Author: Danny Duangphachanh
 *
 * Created on October 4, 2014, 10:03 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H

#define DO_DEBUG

#define I2C_INT_HANDLER LATDbits.LATD6  // DEBUG3 (15)
//#define TIMER0_ISR LATEbits.LATE1       // DEBUG
//#define TIMER1_ISR LATEbits.LATE2       // DEBUG
//#define I2C_SLAVE_SEND LATBbits.LATB2  // DEBUG
#define I2C_SLAVE_RECV LATBbits.LATB1  // DEBUG2 (14)
#define TIMER0 LATDbits.LATD4  // DEBUG4 (16)
#define TIMER1 LATDbits.LATD5  // DEBUG5 (17)

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

