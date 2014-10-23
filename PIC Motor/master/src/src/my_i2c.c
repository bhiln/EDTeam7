#include "maindefs.h"
#ifndef __XC8
#include <i2c.h>
#else
#include <plib/i2c.h>
#endif
#include "my_i2c.h"
#include "debug.h"
#include "messages.h"

static i2c_comm *ic_ptr;

// Configure for I2C Master mode -- the variable "slave_addr" should be stored in
//   i2c_comm (as pointed to by ic_ptr) for later use.

void i2c_configure_master(unsigned char sensor_addr, unsigned char motor_addr) {
    // Your code goes here
    ic_ptr->status = I2C_IDLE;
    SSPSTAT = 0;
    SSPCON1 = 0;
    SSPCON2 = 0;
    SSPCON1bits.SSPM |= 0x8;

    TRISBbits.TRISB4 = 1; // RB4 = SCL1
    TRISBbits.TRISB5 = 1; // RB5 = SDA1

    SSPSTATbits.SMP = 1;
    SSPSTATbits.CKE = 0;
    SSPADD = 0x77;

    ic_ptr->sensor_addr = sensor_addr;
    ic_ptr->motor_addr = motor_addr;
    ic_ptr->buflen = MAXI2CBUF;

    SSPCON1bits.SSPEN = 1;
}

// Sending in I2C Master mode [slave write]
// 		returns -1 if the i2c bus is busy
// 		return 0 otherwise
// Will start the sending of an i2c message -- interrupt handler will take care of
//   completing the message send.  When the i2c message is sent (or the send has failed)
//   the interrupt handler will send an internal_message of type MSGT_MASTER_SEND_COMPLETE if
//   the send was successful and an internal_message of type MSGT_MASTER_SEND_FAILED if the
//   send failed (e.g., if the slave did not acknowledge).  Both of these internal_messages
//   will have a length of 0.
// The subroutine must copy the msg to be sent from the "msg" parameter below into
//   the structure to which ic_ptr points [there is already a suitable buffer there].

unsigned char i2c_master_send(unsigned char addr, unsigned char length, unsigned char *msg) {
    // Your code goes here
    if (SSP1STATbits.R_W == 1) {
        return (-1);
    }
    if (ic_ptr->status == I2C_IDLE) {
        ic_ptr->slave_addr = addr;
        ic_ptr->status = I2C_SEND_START; // set status to start motor transmission
        FromMainHigh_sendmsg(length, MSGT_I2C_MOTOR_CMD, (void *) msg); // puts msg into FromMainHigh_MQ
        ic_ptr->outbufind = 0;
        ic_ptr->outbuflen = length;
        SSP1CON2bits.SEN = 1; // go to i2c_master_int_handler
    }
    return (0);
}

// Receiving in I2C Master mode [slave read]
// 		returns -1 if the i2c bus is busy
// 		return 0 otherwise
// Will start the receiving of an i2c message -- interrupt handler will take care of
//   completing the i2c message receive.  When the receive is complete (or has failed)
//   the interrupt handler will send an internal_message of type MSGT_MASTER_RECV_COMPLETE if
//   the receive was successful and an internal_message of type MSGT_MASTER_RECV_FAILED if the
//   receive failed (e.g., if the slave did not acknowledge).  In the failure case
//   the internal_message will be of length 0.  In the successful case, the
//   internal_message will contain the message that was received [where the length
//   is determined by the parameter passed to i2c_master_recv()].
// The interrupt handler will be responsible for copying the message received into

unsigned char i2c_master_recv(unsigned char addr, unsigned char length) {
    // Your code goes here
    if (SSP1STATbits.R_W == 1) {
        return (-1);
    }
    if (ic_ptr->status == I2C_IDLE) {
        ic_ptr->slave_addr = addr;
        ic_ptr->buflen = length;
        ic_ptr->bufind = 0;
        ic_ptr->status = I2C_RECV_START; // set status to start receiving data from slave devices
        SSP1CON2bits.SEN = 1; // go to i2c_master_int_handler
    }
    return (0);
}

//void start_i2c_slave_reply(unsigned char length, unsigned char *msg) {
//
//    for (ic_ptr->outbuflen = 0; ic_ptr->outbuflen < length; ic_ptr->outbuflen++) {
//        ic_ptr->outbuffer[ic_ptr->outbuflen] = msg[ic_ptr->outbuflen];
//    }
//    ic_ptr->outbuflen = length;
//    ic_ptr->outbufind = 1; // point to the second byte to be sent
//
//    // put the first byte into the I2C peripheral
//    SSPBUF = ic_ptr->outbuffer[0];
//    // we must be ready to go at this point, because we'll be releasing the I2C
//    // peripheral which will soon trigger an interrupt
//    SSPCON1bits.CKP = 1;
//}

// an internal subroutine used in the slave version of the i2c_int_handler

void handle_start(unsigned char data_read) {
    ic_ptr->event_count = 1;
    ic_ptr->buflen = 0;
    // check to see if we also got the address
    if (data_read) {
        if (SSPSTATbits.D_A == 1) {
            // this is bad because we got data and
            // we wanted an address
            ic_ptr->status = I2C_IDLE;
            ic_ptr->error_count++;
            ic_ptr->error_code = I2C_ERR_NOADDR;
        } else {
            if (SSPSTATbits.R_W == 1) {
                ic_ptr->status = I2C_SLAVE_SEND;
            } else {
                ic_ptr->status = I2C_RCV_DATA;
            }
        }
    } else {
        ic_ptr->status = I2C_STARTED;
    }
}

// this is the interrupt handler for i2c -- it is currently built for slave mode
// -- to add master mode, you should determine (at the top of the interrupt handler)
//    which mode you are in and call the appropriate subroutine.  The existing code
//    below should be moved into its own "i2c_slave_handler()" routine and the new
//    master code should be in a subroutine called "i2c_master_handler()"

void i2c_master_int_handler() {
    unsigned char i2c_data;

    switch (ic_ptr->status) {
        // Idle state for i2c communication
        case I2C_IDLE:
        {
            //PIR1bits.SSP1IF = 0;
//            i2c_data = FromMainHigh_recvmsg(ic_ptr->buflen, (void *) MSGT_I2C_DATA, (void *) ic_ptr->buffer);
//            if (i2c_data < 0) {
//                break;
//            } else {
//                ic_ptr->status = I2C_RECV_START; // set status to start receiving data from slave devices
//                SSP1CON2bits.SEN = 1; // go to i2c_master_int_handler
//            }
            break;
        };
        
        // Start receive data case state
        // Sends the address to slave PIC with read bit
        case I2C_RECV_START:
        {
            //PIR1bits.SSP1IF = 0;
            if (ic_ptr->slave_addr == ic_ptr->sensor_addr || ic_ptr->slave_addr == ic_ptr->motor_addr) {
                ic_ptr->status = I2C_RECV_ACK;
                SSP1BUF = ((ic_ptr->slave_addr)) + 1;
            } else {
                ic_ptr->status = I2C_IDLE;
                SSP1CON2bits.PEN = 1;
            }
            break;
        };
        case I2C_RECV_ACK:
        {
            //PIR1bits.SSP1IF = 0;
            if (SSP1CON2bits.ACKSTAT == 0) {
                ic_ptr->status = I2C_RECV_DATA;
                SSP1CON2bits.RCEN = 1;
                SSP1CON2bits.SEN = 1; // do i need this enable bit?
            } else {
                ic_ptr->status = I2C_IDLE;
                ToMainHigh_sendmsg(0, MSGT_I2C_MASTER_RECV_FAILED, 0);
                SSP1CON2bits.PEN = 1;
                // use LED to show NACK in main case
            }
            break;
        };
        case I2C_RECV_DATA:
        {
            //PIR1bits.SSP1IF = 0;
            if (ic_ptr->bufind != (ic_ptr->buflen - 1)) {
                ic_ptr->status = I2C_RECV_RCEN;
                ic_ptr->buffer[ic_ptr->bufind] = SSP1BUF;
                ic_ptr->bufind++;
                SSP1CON2bits.ACKEN = 1;
                SSP1CON2bits.ACKDT = 0;
                SSP1CON2bits.SEN = 1;  // do i need this enable bit?
            } else {
                ic_ptr->status = I2C_RECV_STOP;
                SSP1CON2bits.ACKEN = 1;
                SSP1CON2bits.ACKDT = 1;
                SSP1CON2bits.SEN = 1;  // do i need this enable bit?
            }
            break;
        };
        case I2C_RECV_RCEN:
        {
            //PIR1bits.SSP1IF = 0;
            ic_ptr->status = I2C_RECV_DATA;
            SSP1CON2bits.RCEN = 1;
            SSP1CON2bits.SEN = 1;  // do i need this enable bit?
            break;
        };
        case I2C_RECV_STOP:
        {
            //PIR1bits.SSP1IF = 0;
            ic_ptr->status = I2C_IDLE;
            ToMainHigh_sendmsg(ic_ptr->buflen, MSGT_I2C_MASTER_RECV_COMPLETE, (void *) ic_ptr->buffer);
            SSP1CON2bits.PEN = 1;
            break;
        };

        
        // Start send data case state
        // Sends the address to slave PIC with write bit
        case I2C_SEND_START:
        {
            //PIR1bits.SSP1IF = 0; // clear IF
            ic_ptr->status = I2C_SEND_DATA; // set to Motor send state
            SSP1BUF = ((ic_ptr->slave_addr) + 0);
            break;
        };
        case I2C_SEND_DATA: // Motor send state
        {
            //PIR1bits.SSP1IF = 0; // clear IF
            if (SSP1CON2bits.ACKDT == 0) {
                SSP1CON2bits.ACKSTAT = 0; // clear ACK bit
                FromMainHigh_recvmsg(ic_ptr->outbuflen, (void *) MSGT_I2C_MOTOR_CMD, (void *) ic_ptr->outbuffer);
                if (ic_ptr->outbufind < ic_ptr->outbuflen) {
                    ic_ptr->status = I2C_SEND_DATA;
                    ic_ptr->outbufind++;
                    SSP1BUF = ic_ptr->outbuffer[ic_ptr->outbufind - 1];
                } else if (ic_ptr->outbufind == ic_ptr->outbuflen) {
                    ToMainHigh_sendmsg(0, MSGT_I2C_MASTER_SEND_COMPLETE, 0);
                    ic_ptr->status = I2C_IDLE;
                    ic_ptr->outbuflen = 0;
                    SSP1CON2bits.PEN = 1;
                } else {
                    ToMainHigh_sendmsg(ic_ptr->outbuflen, MSGT_I2C_MASTER_SEND_FAILED,  (void *) ic_ptr->outbuffer);
                    ic_ptr->status = I2C_IDLE;
                    SSP1CON2bits.ACKSTAT = 1;
                    SSP1CON2bits.PEN = 1;
                }
            } else {
                ToMainHigh_sendmsg(ic_ptr->outbuflen, MSGT_I2C_MASTER_SEND_FAILED,  (void *) ic_ptr->outbuffer);
                ic_ptr->status = I2C_IDLE;
                SSP1CON2bits.ACKSTAT = 1;
                SSP1CON2bits.PEN = 1;
            }
            break;
        };

//        // Case statements reached end
//        case I2C_STOPPED:
//        {
//            PIR1bits.SSP1IF = 0;
//            ic_ptr->status = I2C_IDLE;
//            SSP1CON2bits.PEN = 1;
//            break;
//        };
    }
}

// set up the data structures for this i2c code
// should be called once before any i2c routines are called

void init_i2c(i2c_comm *ic) {
    ic_ptr = ic;
    ic_ptr->buflen = 0;
    ic_ptr->event_count = 0;
    ic_ptr->status = I2C_IDLE;
    ic_ptr->error_count = 0;
}