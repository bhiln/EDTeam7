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
#define MSGT_MAIN1 10
#define MSGT_TIMER0 20
#define MSGT_I2C_DBG 21
#define MSGT_I2C_RQST 22
#define MSGT_I2C_SLAVE_RECV_COMPLETE 23
#define MSGT_I2C_DATA 24
#define MSGT_TIMER1 30
#define	MSGT_OVERRUN 31
#define MSGT_UART_DATA 32
#define MSGT_UART_CMD 33

#endif

