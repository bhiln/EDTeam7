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
} i2c_comm;

#define I2C_IDLE 0x4
#define I2C_STARTED 0x5
#define I2C_RCV_DATA 0x6
#define	I2C_RCV_SENSOR_DATA 0x7
#define I2C_RCV_QE_DATA 0x8
#define I2C_SLAVE_SEND 0x9
#define I2C_SEND_ADDR 0xA
#define I2C_SEND_DATA 0xB
#define I2C_STOPPED 0xC
#define I2C_ACK 0xD
#define I2C_RCV_SENSOR_ADDR 30
#define I2C_RCV_SENSOR_ACK 31
#define I2C_RCV_SENSOR_RELOAD 32
#define I2C_RCV_SENSOR_EN 33
#define I2C_RCV_SENSOR_STOP 34
#define I2C_SEND_MOTOR_ADDR 40
#define I2C_SEND_MOTOR_DATA 41
#define I2C_SEND_MOTOR_NEXT 42
#define I2C_SEND_MOTOR_STOP 43


#define I2C_ERR_THRESHOLD 20 //1
#define I2C_ERR_OVERRUN 21 //0x4
#define I2C_ERR_NOADDR 22 //0x5
#define I2C_ERR_NODATA 23 //0x6
#define I2C_ERR_MSGTOOLONG 24 //0x7
#define I2C_ERR_MSG_TRUNC 25 //0x8

void init_i2c(i2c_comm *);
void i2c_int_handler(void);
void start_i2c_slave_reply(unsigned char,unsigned char *);
void i2c_configure_slave(unsigned char);
void i2c_configure_master(unsigned char);
unsigned char i2c_master_send(unsigned char,unsigned char *,unsigned char);
unsigned char i2c_master_recv(unsigned char,unsigned char);

#endif