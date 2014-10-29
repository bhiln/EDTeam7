#include "maindefs.h"
#include <stdio.h>
#ifndef __XC8
#include <usart.h>
#include <i2c.h>
#include <timers.h>
#else
#include <plib/usart.h>
#include <plib/i2c.h>
#include <plib/timers.h>
#endif
#include "interrupts.h"
#include "messages.h"
#include "my_uart.h"
#include "my_i2c.h"
#include "uart_thread.h"
#include "timer1_thread.h"
#include "timer0_thread.h"
#include "ADC.h"
#include "debug.h"

// Setup configuration registers
// ARM PIC (slave)
#ifdef __USE18F46J50

// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 3       // PLL Prescaler Selection bits (Divide by 3 (12 MHz oscillator input))
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset (Disabled)
#ifndef __XC8
// Have to turn this off because I don't see how to enable this in the checkboxes for XC8 in this IDE
#pragma config XINST = ON       // Extended Instruction Set (Enabled)
#else
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)
#endif

// CONFIG1H
#pragma config CPUDIV = OSC1    // CPU System Clock Postscaler (No CPU system clock divide)
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)

// CONFIG2L
#pragma config OSC = HSPLL      // Oscillator (HS+PLL, USB-HS+PLL)
#pragma config T1DIG = OFF      // T1OSCEN Enforcement (Secondary Oscillator clock source may not be selected)
#pragma config LPT1OSC = OFF    // Low-Power Timer1 Oscillator (High-power operation)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO = ON        // Internal External Oscillator Switch Over Mode (Enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler (1:32768)

// CONFIG3L
#pragma config DSWDTOSC = T1OSCREF// DSWDT Clock Select (DSWDT uses T1OSC/T1CKI)
#pragma config RTCOSC = T1OSCREF// RTCC Clock Select (RTCC uses T1OSC/T1CKI)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))

// CONFIG3H
#pragma config IOL1WAY = ON     // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set once)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)

// CONFIG4L
#pragma config WPFP = PAGE_63   // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 63)
#pragma config WPEND = PAGE_WPFP// Write/Erase Protect Region Select (valid when WPDIS = 0) (Page WPFP<5:0> through Configuration Words erase/write protected)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region (Configuration Words page not erase/write-protected)

// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<5:0>/WPEND region ignored)

#endif

void main(void) {
    char c;
    signed char length;
    signed char rqst;
    unsigned char msgtype;
    unsigned char last_reg_recvd;
    uart_comm uc;
    i2c_comm ic;
    unsigned char msgbuffer[MSGLEN + 1];
    unsigned char i;
    int k = 0;
    uart_thread_struct uthread_data; // info for uart_lthread
    timer1_thread_struct t1thread_data; // info for timer1_lthread
    timer0_thread_struct t0thread_data; // info for timer0_lthread

    OSCCON = 0xE0; //see datasheet
    OSCTUNEbits.PLLEN = 1;

    // initialize my uart recv handling code
    init_uart_recv(&uc);

    // initialize the i2c code
    init_i2c(&ic);

    // init the timer1 lthread
    init_timer1_lthread(&t1thread_data);

    // initialize message queues before enabling any interrupts
    init_queues();

    // how to set up PORTA for input (for the V4 board with the PIC2680)
    /*
            PORTA = 0x0;	// clear the port
            LATA = 0x0;		// clear the output latch
            ADCON1 = 0x0F;	// turn off the A2D function on these pins
            // Only for 40-pin version of this chip CMCON = 0x07;	// turn the comparator off
            TRISA = 0x0F;	// set RA3-RA0 to inputs
     */

    //Initialize the ADC module
    ADC_Init();

    // initialize Timers
    OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_128);
    OpenTimer1(TIMER_INT_ON & T1_SOURCE_FOSC_4 & T1_PS_1_8 & T1_16BIT_RW & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF,0x0);

    // Decide on the priority of the enabled peripheral interrupts
    // 0 is low, 1 is high
    // Timer0 interrupt
    INTCON2bits.TMR0IP = 0;
    // Timer1 interrupt
    IPR1bits.TMR1IP = 0;
    // USART RX interrupt
    IPR1bits.RCIP = 0;
    // USART TX interrupt
    IPR1bits.TXIP = 0;
    // I2C interrupt
    IPR1bits.SSPIP = 1;

    // configure the hardware i2c device as a slave (0x9E -> 0x4F) or (0x9A -> 0x4D)
#if 1
    // Note that the temperature sensor Address bits (A0, A1, A2) are also the
    // least significant bits of LATB -- take care when changing them
    // They *are* changed in the timer interrupt handlers if those timers are
    //   enabled.  They are just there to make the lights blink and can be
    //   disabled.
     i2c_configure_slave(0x9E);

#else
    // If I want to test the temperature sensor from the ARM, I just make
    // sure this PIC does not have the same address and configure the
    // temperature sensor address bits and then just stay in an infinite loop
    i2c_configure_slave(0x9A);
    
#endif

    // must specifically enable the I2C interrupts
    PIE1bits.SSPIE = 1;

    // configure the hardware USART device
    Open1USART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
        USART_CONT_RX & USART_BRGH_LOW, 0x19);

    // Peripheral interrupts can have their priority set to high or low
    // enable high-priority interrupts and low-priority interrupts
    enable_interrupts();

    DEBUG_OFF(I2C_SLAVE_REPLY);
    //Initialize
    TRISEbits.TRISE0 = 0x0;

    TRISCbits.TRISC7 = 0x1; // input RX
    TRISCbits.TRISC6 = 0x0; // output TX

    SPBRGH1 = 0x00;
    SPBRG1 = 0xCF;

    TXSTA1bits.BRGH = 1;
    BAUDCON1bits.BRG16 = 1;
    TXSTA1bits.SYNC = 0;
    RCSTA1bits.SPEN = 0x1;
    TXSTA1bits.TXEN = 0x1;

    // printf() is available, but is not advisable.  It goes to the UART pin
    // on the PIC and then you must hook something up to that to view it.
    // It is also slow and is blocking, so it will perturb your code's operation
    // Here is how it looks: printf("Hello\r\n");

    // loop forever
    // This loop is responsible for "handing off" messages to the subroutines
    // that should get them.  Although the subroutines are not threads, but
    // they can be equated with the tasks in your task diagram if you
    // structure them properly.

    while (1) {
        // Call a routine that blocks until either on the incoming
        // messages queues has a message (this may put the processor into
        // an idle mode)
        block_on_To_msgqueues();

        // At this point, one or both of the queues has a message.  It
        // makes sense to check the high-priority messages first -- in fact,
        // you may only want to check the low-priority messages when there
        // is not a high priority message.  That is a design decision and
        // I haven't done it here.
        length = ToMainHigh_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message, check the error code to see if it is concern
            if (length != MSGQUEUE_EMPTY) {
                // This case be handled by your code.
            }
        } else {
            switch (msgtype) {
                case MSGT_TIMER0: {
                    timer0_lthread(&t0thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_I2C_DBG: {
                    // Here is where you could handle debugging, if you wanted
                    // keep track of the first byte received for later use (if desired)
                    last_reg_recvd = msgbuffer[0];
                    break;
                };
                case MSGT_I2C_RQST:
                {
                    DEBUG_ON(I2C_SLAVE_REPLY);
//                    rqst = FromMainLow_recvmsg(length, MSGT_I2C_DATA, (void *) msgbuffer);
//                    if (rqst > 0) {
//                        start_i2c_slave_reply(2, msgbuffer);
//                    }

                    unsigned char forward[22];
                    length = 22;
                    forward[0] = 0x11;
                    forward[1] = 0x12;
                    forward[2] = 0x13;
                    forward[3] = 0x14;
                    forward[4] = 0x15;
                    forward[5] = 0x16;
                    forward[6] = 0x17;
                    forward[7] = 0x18;
                    forward[8] = 0x19;
                    forward[9] = 0xAA;
                    forward[10] = 0xBB;
                    forward[11] = 0xCC;
                    forward[12] = 0xDD;
                    forward[13] = 0xEE;
                    forward[14] = 0x21;
                    forward[15] = 0x22;
                    forward[16] = 0x23;
                    forward[17] = 0x24;
                    forward[18] = 0x25;
                    forward[19] = 0x26;
                    forward[20] = 0x27;
                    forward[21] = 0x28;
                    // Send UART to ARM
//                    ToMainHigh_recvmsg(5, MSGT_I2C_DATA, msgbuffer);
//                    start_i2c_slave_reply(length, msgbuffer);
                    start_i2c_slave_reply(22, forward);
                    DEBUG_OFF(I2C_SLAVE_REPLY);
                    break;
                };
                case MSGT_I2C_SLAVE_RECV_COMPLETE:
                {
                    // have case statements for motor commands
                    // FromMainHigh_sendmsg(6, MSGT_UART_CMD, msgbuffer);

//                    unsigned char forward[5];
//                    length = 5;
//                    forward[0] = 0x0A;
//                    forward[1] = 0x0B;
//                    forward[2] = 0x0C;
//                    forward[3] = 0x0D;
//                    forward[4] = 0x0E;
//                    start_i2c_slave_reply(length, forward);
//                    FromMainLow_recvmsg(length, MSGT_I2C_DATA, (void *) msgbuffer);
//                    start_i2c_slave_reply(2, msgbuffer);
                    // Send Motor Commands to UART
//                    uart_send(1, msgbuffer);
                    break;
                }
                default: {
                    // Your code should handle this error
                    break;
                };
            };
        }

        // Check the low priority queue
        length = ToMainLow_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
        if (length < 0) {
            // no message, check the error code to see if it is concern
            if (length != MSGQUEUE_EMPTY) {
                // Your code should handle this situation
            }
        } else {
            switch (msgtype) {
                case MSGT_TIMER1: {
                    timer1_lthread(&t1thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_OVERRUN: {
                    // We've overrun the USART
                    break;
                };
                case MSGT_UART_DATA: {
//                    ToMainLow_recvmsg(length, MSGT_UART_DATA, msgbuffer);
                    // Send Motor Commands to UART
//                    uart_send(1, msgbuffer);
//                    uart_lthread(&uthread_data, msgtype, length, msgbuffer);
                    // Send UART data to high priority MQ
                    unsigned char uartmsg;
                    // ToMainLow_recvmsg(length, MSGT_UART_DATA, (void *) msgbuffer);
                    uartmsg = msgbuffer;
                    ToMainHigh_sendmsg(length, MSGT_I2C_RQST, uartmsg);
                    break;
                };
                case MSGT_UART_CMD: {
                    // ToMainLow_recvmsg(length, MSGT_UART_CMD, (void *) msgbuffer);
                    // Send Motor Commands to UART
                    uart_send(1, msgbuffer);
                    break;
                }
                default: {
                    // Your code should handle this error
                    break;
                };
            };
        }
    }

}