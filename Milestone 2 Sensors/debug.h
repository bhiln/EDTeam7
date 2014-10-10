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
#define ADC_START LATBbits.LATB1
#define I2C_ISR LATBbits.LATB2
//#define ADC_ISR LATBbits.LATB0
//#define ADC_START LATBbits.LATB1
//#define I2C_ISR LATBbits.LATB2
#define SAVE_FROM_MASTER LATDbits.LATD5
#define I2C_SLAVE_REPLY LATBbits.LATB5
#define ADC_MSG_SEND LATDbits.LATD7
#define ADC_MSG_RCV LATDbits.LATD6
//#define TIMER0_ISR LATBbits.LATB6


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

