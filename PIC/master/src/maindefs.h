#ifndef __maindefs
#define __maindefs

#ifdef __XC8
#include <xc.h>
#ifdef _18F45J10
#define __USE18F45J10 1
#else
#ifdef _18F2680
#define __USE18F2680 1
#else
#ifdef _18F26J50
#define __USE18F26J50 1
#else
#ifdef _18F46J50
#define __USE18F46J50 1
#endif
#endif
#endif
#endif
#else
#ifdef __18F45J10
#define __USE18F45J10 1
#else
#ifdef __18F2680
#define __USE18F2680 1
#else
#ifdef __18F26J50
#define __USE18F26J50 1
#else
#ifdef __18F46J50
#define __USE18F46J50 1
#endif
#endif
#endif
#endif
#include <p18cxxx.h>
#endif

// Message type definitions
#define MSGT_TIMER0 10
#define MSGT_TIMER1 11
#define MSGT_MAIN1 20
#define	MSGT_OVERRUN 30
#define MSGT_UART_DATA 31
#define MSGT_I2C_DBG 40
#define	MSGT_I2C_DATA 41
#define	MSGT_I2C_SENSOR_DATA 44
#define MSGT_I2C_MOTOR_DATA 45
#define MSGT_I2C_MOTOR_CMD 46
#define MSGT_I2C_RQST 47
#define MSGT_I2C_MASTER_SEND_COMPLETE 50
#define MSGT_I2C_MASTER_SEND_FAILED 51
#define MSGT_I2C_MASTER_RECV_COMPLETE 52
#define MSGT_I2C_MASTER_RECV_FAILED 53

#endif

