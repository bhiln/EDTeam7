#ifndef __my_i2c_h
#define __my_i2c_h

#include "messages.h"

#define MAXI2CBUF MSGLEN
typedef struct __i2c_comm {
    unsigned char buffer[MAXI2CBUF];
    unsigned char buflen;
    unsigned char bufind;
    unsigned char event_count;
    unsigned char status;
    unsigned char error_code;
    unsigned char error_count;
    unsigned char outbuffer[MAXI2CBUF];
    unsigned char outbuflen;
    unsigned char outbufind;
    unsigned char slave_addr;
    unsigned char sensor_addr;
    unsigned char motor_addr;
    unsigned char msg_count;
} i2c_comm;

#define I2C_IDLE 0x1
#define I2C_STARTED 0x2
#define	I2C_RCV_DATA 0x3
#define I2C_SLAVE_SEND 0x4
#define I2C_STOPPED 0x5
#define I2C_SEND_START 0x6
#define I2C_SEND_DATA 0x7
#define I2C_RECV_START 0x8
#define I2C_RECV_ACK 0x9
#define I2C_RECV_DATA 0xA
#define I2C_RECV_RCEN 0xB
#define I2C_RECV_STOP 0xC

#define I2C_ERR_THRESHOLD 20
#define I2C_ERR_OVERRUN 21
#define I2C_ERR_NOADDR 22
#define I2C_ERR_NODATA 23
#define I2C_ERR_MSGTOOLONG 24
#define I2C_ERR_MSG_TRUNC 25
#define I2C_ERR_NOACK 26

void init_i2c(i2c_comm *);
void i2c_int_handler(void);
void i2c_master_int_handler(void);
void i2c_slave_int_handler(void);
void start_i2c_slave_reply(unsigned char,unsigned char *);
void i2c_configure_slave(unsigned char);
void i2c_configure_master(unsigned char,unsigned char);
unsigned char i2c_master_send(unsigned char,unsigned char,unsigned char *);
unsigned char i2c_master_recv(unsigned char,unsigned char);

#endif