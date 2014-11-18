/* 
 * File:   debug.h
 * Author: Danny Duangphachanh
 *
 * Created on October 4, 2014, 10:03 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H

#define DO_DEBUG

#define I2C_INT_HANDLER LATEbits.LATE0  // DEBUG8
#define TIMER0_ISR LATEbits.LATE1       // DEBUG6
#define TIMER1_ISR LATEbits.LATE2       // DEBUG4
//#define I2C_MASTER_SEND LATAbits.LATA2  // DEBUG7
//#define I2C_MASTER_RECV LATAbits.LATA3   // DEBUG5
#define UART_TX LATAbits.LATA3  // DEBUG7
#define UART_RX LATAbits.LATA2   // DEBUG5

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

