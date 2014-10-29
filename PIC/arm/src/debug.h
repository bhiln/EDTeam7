/* 
 * File:   debug.h
 * Author: Leah
 *
 * Created on September 18, 2014, 8:38 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H

#define DO_DEBUG

#define TIMER0_MSG_SEND LATBbits.LATB0
#define TIMER0_MSG_RCV LATBbits.LATB1
#define TIMER0_ISR LATBbits.LATB2
#define TIMER1_MSG_SEND LATDbits.LATD4
#define TIMER1_MSG_RCV LATDbits.LATD5
#define TIMER1_ISR LATDbits.LATD6
#define I2C_MASTER_SEND LATEbits.LATE0
#define I2C_ISR LATEbits.LATE2
#define I2C_INT_HANDLER LATEbits.LATE1
#define ADC_ISR LATDbits.LATD2
#define ADC_START LATDbits.LATD3
#define ADC_READ LATCbits.LATC0


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
