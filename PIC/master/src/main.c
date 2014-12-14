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
#include "debug.h"

// Setup configuration registers
// Rover Master PIC
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
    signed char length;
    unsigned char msgtype;
    unsigned char last_reg_recvd;
    unsigned char speed;
    unsigned char distance;
    unsigned char command;
    unsigned char done_msg[1];
    unsigned char done_length;
    uart_comm uc;
    i2c_comm ic;
    unsigned char msgbuffer[MSGLEN + 1];
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

    // initialize Timers
    OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_64);
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

    // configure the hardware i2c device as a master.
    // sensor slave pic (0x9E -> 0x4F) && motor slave pic (0x9A -> 0x4D)
    i2c_configure_master(0x9E, 0x9A); // (sensor address, motor address)

    // must specifically enable the I2C interrupts
    PIE1bits.SSPIE = 1;

    // configure the hardware USART device
    Open1USART(USART_TX_INT_OFF & USART_RX_INT_ON & USART_ASYNCH_MODE & USART_EIGHT_BIT &
        USART_CONT_RX & USART_BRGH_LOW, 0x19);

    // Peripheral interrupts can have their priority set to high or low
    // enable high-priority interrupts and low-priority interrupts
    enable_interrupts();

    // Initialize
    TRISEbits.TRISE0 = 0x0;
    TRISEbits.TRISE1 = 0x0;
    TRISEbits.TRISE2 = 0x0;
    TRISBbits.TRISB0 = 0x0; // debug pin
    TRISBbits.TRISB1 = 0x0; // debug pin
    TRISBbits.TRISB2 = 0x0; // debug pin
    TRISBbits.TRISB3 = 0x0; // debug pin
    TRISAbits.TRISA2 = 0x0; // led0
    TRISAbits.TRISA3 = 0x0; // led1
    TRISCbits.TRISC7 = 0x1; // input UART RX
    TRISCbits.TRISC6 = 0x0; // output UART TX

    SPBRGH1 = 0x00;
    SPBRG1 = 0xCF;

    TXSTA1bits.BRGH = 1;
    BAUDCONbits.BRG16 = 1;
    TXSTA1bits.SYNC = 0;
    RCSTA1bits.SPEN = 0x1;
    TXSTA1bits.TXEN = 0x1;

    // printf() is available, but is not advisable.  It goes to the UART pin
    // on the PIC and then you must hook something up to that to view it.
    // It is also slow and is blocking, so it will perturb your code's operation
    // Here is how it looks: printf("Hello\r\n");

//    DEBUG_OFF(UART_TX);
//    DEBUG_OFF(UART_RX);
    DEBUG_OFF(I2C_START);
    DEBUG_OFF(I2C_ACK);
    DEBUG_OFF(I2C_RECV);
    DEBUG_OFF(I2C_STOP);

//    i2c_master_recv(0x9E, 20);
//    unsigned char i2cmsg[3];
//    unsigned char i2clen = 3;
//    i2cmsg[0] = 0x0B; // command
//    i2cmsg[1] = 20; // distance
//    i2cmsg[2] = 32; // speed
//    i2c_master_send(0x9A, i2clen, i2cmsg);
    
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
                    timer0_lthread(&t0thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_I2C_DATA:
                case MSGT_I2C_DBG:
                {
                    // Here is where you could handle debugging, if you wanted
                    // keep track of the first byte received for later use (if desired)
                    last_reg_recvd = msgbuffer[0];
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

                };
                case MSGT_I2C_MASTER_RECV_COMPLETE:
                {
                    // Send sensor/QE data to ARM
                    if (length == 1) { // motor data
                        if (msgbuffer[0] == 0x35) {
                            done_msg[0] = 0x1;
                            MotorData_sendmsg(1, MSGT_I2C_MOTOR_DATA, (void *) done_msg);
                        }
//                        else {
//                            done_msg[0] = 0x0;
//                            MotorData_sendmsg(1, MSGT_I2C_MOTOR_DATA, (void *) done_msg);
//                        }
//                        else if (msgbuffer[1] == 0x34) {
//                            done_msg[0] = 2;
//                            MotorData_sendmsg(1, MSGT_I2C_MOTOR_DATA, (void *) done_msg);
//                        }
//                        uart_send(length, msgbuffer);
                    }
                    if (length == 23) { // sensor data
                        done_length = MotorData_recvmsg(1, (void *) MSGT_I2C_MOTOR_DATA, (void *) done_msg);
                        if (done_length == 1) {
                            msgbuffer[22] = done_msg[0];
                            done_length = 0;
                        } else {
                            msgbuffer[22] = 0x0;
                        }
                        msgbuffer[23] = 0xFF;
                        uart_send(24, msgbuffer);
                    }
                    break;
                };
                case MSGT_I2C_MOTOR_CMD:
                {
                    command = msgbuffer[2];
                    distance = msgbuffer[3];
                    speed = msgbuffer[4];
                    switch (command) {
                        case 0x0A: // forward
                        {
                            length = 3;
                            msgbuffer[0] = 0x0A; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0B: // turn left
                        {
                            length = 3;
                            msgbuffer[0] = 0x0B; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0C: // turn right
                        {
                            length = 3;
                            msgbuffer[0] = 0x0C; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0D: // stop
                        {
                            length = 3;
                            msgbuffer[0] = 0x0D; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0E: // reverse
                        {
                            length = 3;
                            msgbuffer[0] = 0x0E; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                    };
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
        }
        else {
            switch (msgtype) {
                case MSGT_TIMER1:
                {
                    timer1_lthread(&t1thread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_OVERRUN:
                {
                    uart_lthread(&uthread_data, msgtype, length, msgbuffer);
                    break;
                };
                case MSGT_UART_DATA:
                {
                    command = msgbuffer[2];
                    distance = msgbuffer[3];
                    speed = msgbuffer[4];

                    switch (command) {
                        case 0x0A: // forward
                        {
                            length = 3;
                            msgbuffer[0] = 0x0A; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
//                            i2c_master_recv(0x9A, 4);
                            break;
                        }
                        case 0x0B: // turn left
                        {
                            length = 3;
                            msgbuffer[0] = 0x0B; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0C: // turn right
                        {
                            length = 3;
                            msgbuffer[0] = 0x0C; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0D: // stop
                        {
                            length = 3;
                            msgbuffer[0] = 0x0D; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                        case 0x0E: // reverse
                        {
                            length = 3;
                            msgbuffer[0] = 0x0E; // command
                            msgbuffer[1] = distance; // distance
                            msgbuffer[2] = speed; // speed
                            i2c_master_send(0x9A, length, msgbuffer);
                            break;
                        }
                    };
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