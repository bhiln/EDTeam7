#include "maindefs.h"
#include <stdio.h>
#include <math.h>
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
#include "user_interrupts.h"
#include "uart_thread.h"
#include "timer1_thread.h"
#include "timer0_thread.h"
#include "debug.h"

// Setup configuration registers
// Motor Slave PIC
#ifdef __USE18F45J10

// CONFIG1L
#pragma config WDTEN = OFF      // Watchdog Timer Enable bit (WDT disabled (control is placed on SWDTEN bit))
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable bit (Reset on stack overflow/underflow disabled)
#ifndef __XC8
// Have to turn this off because I don't see how to enable this in the checkboxes for XC8 in this IDE
#pragma config XINST = ON       // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)
#else
#pragma config XINST = OFF      // Extended Instruction Set Enable bit (Instruction set extension and Indexed Addressing mode enabled)
#endif

// CONFIG1H
#pragma config CP0 = OFF        // Code Protection bit (Program memory is not code-protected)

// CONFIG2L
#pragma config FOSC = HSPLL     // Oscillator Selection bits (HS oscillator, PLL enabled and under software control)
#pragma config FOSC2 = ON       // Default/Reset System Clock Select bit (Clock selected by FOSC as system clock is enabled when OSCCON<1:0> = 00)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Two-Speed Start-up (Internal/External Oscillator Switchover) Control bit (Two-Speed Start-up enabled)

// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Timer Postscale Select bits (1:32768)

// CONFIG3H
#pragma config CCP2MX = DEFAULT // CCP2 MUX bit (CCP2 is multiplexed with RC1)

#endif

void main(void) {
    signed char length;
    unsigned char msgtype;
    unsigned char last_reg_recvd;
    unsigned char command = 0x0D;
    unsigned char speed = 0;
    uart_comm uc;
    i2c_comm ic;
    tmr0_comm t0;
    unsigned char msgbuffer[MSGLEN + 1];
    uart_thread_struct uthread_data; // info for uart_lthread
    timer1_thread_struct t1thread_data; // info for timer1_lthread
    timer0_thread_struct t0thread_data; // info for timer0_lthread

    OSCCON = 0x82; // see datasheeet
    OSCTUNEbits.PLLEN = 0; // Makes the clock exceed the PIC's rated speed if the PLL is on

    // initialize my uart recv handling code
    init_uart_recv(&uc);

    // initialize the i2c code
    init_i2c(&ic);

    // initialize the tmr0 interrupt code
    init_tmr0(&t0);

    // init the timer1 lthread
    init_timer1_lthread(&t1thread_data);

    // initialize message queues before enabling any interrupts
    init_queues();

    // initialize Timers
    OpenTimer0(TIMER_INT_ON & T0_8BIT & T0_SOURCE_EXT & T0_PS_1_1 & T0_EDGE_FALL & T0_EDGE_RISE);
    OpenTimer1(TIMER_INT_ON & T1_PS_1_8 & T1_16BIT_RW & T1_SOURCE_INT & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF);

    // Decide on the priority of the enabled peripheral interrupts
    // 0 is low, 1 is high
    // Timer0 interrupt
//    INTCON2bits.TMR0IP = 0;
    // Timer1 interrupt
    IPR1bits.TMR1IP = 0;
    // USART RX interrupt
    IPR1bits.RCIP = 0;
    // USART TX interrupt
    IPR1bits.TXIP = 0;
    // I2C interrupt
    IPR1bits.SSPIP = 1;

    // configure the hardware i2c device as a slave (0x9E -> 0x4F) or (0x9A -> 0x4D)
    i2c_configure_slave(0x9A); // 0011 0100 ----- 1001 1010 ----- 0100 1101

    // must specifically enable the I2C interrupts
    PIE1bits.SSPIE = 1;

    // configure the hardware USART device
    OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
        USART_CONT_RX & USART_BRGH_LOW, 0x19);

    // Peripheral interrupts can have their priority set to high or low
    // enable high-priority interrupts and low-priority interrupts
    enable_interrupts();

    // Initialize
    SPBRGH = 0x01; // 0x01;
    SPBRG = 0x37; // 0x37;
    TXSTAbits.BRGH = 0x1;
    BAUDCONbits.BRG16 = 0x1;
    TXSTAbits.SYNC = 0x0;
    RCSTAbits.SPEN = 0x1;
    PIE1bits.TXIE = 0x0;
    TXSTAbits.TXEN = 0x1;

    TRISBbits.TRISB1 = 0x0;
    TRISDbits.TRISD4 = 0x0; // output debug pin
    TRISDbits.TRISD5 = 0x0; // output debug pin
    TRISDbits.TRISD6 = 0x0; // output debug pin
    TRISCbits.TRISC7 = 0x1; // input RX
    TRISCbits.TRISC6 = 0x0; // output TX
    TRISBbits.TRISB5 = 0x1; // RB5 = T0CKI

    // printf() is available, but is not advisable.  It goes to the UART pin
    // on the PIC and then you must hook something up to that to view it.
    // It is also slow and is blocking, so it will perturb your code's operation
    // Here is how it looks: printf("Hello\r\n");

DEBUG_OFF(I2C_INT_HANDLER);
DEBUG_OFF(I2C_SLAVE_RECV);
DEBUG_OFF(TIMER0);
DEBUG_OFF(TIMER1);

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
                case MSGT_TIMER0:
                {
//                    timer0_lthread(&t0thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_I2C_DATA:
                {
                    break;
                };
                case MSGT_I2C_DBG:
                {
                    // Here is where you could handle debugging, if you wanted
                    // keep track of the first byte received for later use (if desired)
                    last_reg_recvd = msgbuffer[0];
                    break;
                };
                case MSGT_I2C_SLAVE_RECV_COMPLETE:
                {
                    //   [0-127] - [left]  (64 is stop)
                    // [128-255] - [right] (192 is stop)
                    t0.count = 0;
                    command = msgbuffer[0];
                    speed = msgbuffer[2];
                    switch (command) {
                        case 0x0A: // forward
                        {
                            length = 2;
                            t0.distance = msgbuffer[1];
                            msgbuffer[0] = (64 + speed);
                            msgbuffer[1] = (192 + speed);
                            uart_send(length, msgbuffer); // send motor command to motor controller
                            break;
                        }
                        case 0x0B: // turn left
                        {
                            length = 2;
                            t0.distance = msgbuffer[1];
                            msgbuffer[0] = (64 - speed);
                            msgbuffer[1] = (192 + speed);
                            uart_send(length, msgbuffer); // send motor command to motor controller
                            break;
                        }
                        case 0x0C: // turn right
                        {
                            length = 2;
                            t0.distance = msgbuffer[1];
                            msgbuffer[0] = (64 + speed);
                            msgbuffer[1] = (192 - speed);
                            uart_send(length, msgbuffer); // send motor command to motor controller
                            break;
                        }
                        case 0x0D: // stop
                        {
                            length = 2;
                            msgbuffer[0] = 64;
                            msgbuffer[1] = 192;
                            uart_send(length, msgbuffer); // send motor command to motor controller
                            break;
                        }
                        case 0x0E: // reverse
                        {
                            length = 2;
                            t0.distance = (msgbuffer[1] + 1);
                            msgbuffer[0] = (64 - speed);
                            msgbuffer[1] = (192 - speed);
                            uart_send(length, msgbuffer); // send motor command to motor controller
                            break;
                        }
                    };
                    break;
                };
                case MSGT_I2C_RQST:
                {
                    // Generally, this is *NOT* how I recommend you handle an I2C slave request
                    // I recommend that you handle it completely inside the i2c interrupt handler
                    // by reading the data from a queue (i.e., you would not send a message, as is done
                    // now, from the i2c interrupt handler to main to ask for data).
                    //
                    // The last byte received is the "register" that is trying to be read
                    // The response is dependent on the register.

//                    length = MotorData_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
//                    if (length > 0) {
//                        start_i2c_slave_reply(3, msgbuffer);
//                        break;
//                    }
                    break;
                };
                default:
                {
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
                case MSGT_TIMER1:
                {
                    timer1_lthread(&t1thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_OVERRUN:
                case MSGT_UART_DATA:
                {
                    uart_lthread(&uthread_data, msgtype, length, msgbuffer);
                    break;
                };
                default:
                {
                    // Your code should handle this error
                    break;
                };
            };
        }
    }

}