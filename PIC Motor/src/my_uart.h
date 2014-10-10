<<<<<<< HEAD:PIC Motor/src/my_uart.h
#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MAXUARTBUF 6
#if (MAXUARTBUF > MSGLEN)
#define MAXUARTBUF MSGLEN
#endif
typedef struct __uart_comm {
    unsigned char buffer[MAXUARTBUF];
    unsigned char buflen;
    unsigned char bufind;
    unsigned char outbuffer[MAXUARTBUF];
    unsigned char outbuflen;
    unsigned char outbufind;
} uart_comm;

void init_uart_recv(uart_comm *);
void uart_trans_int_handler(void);
void uart_recv_int_handler(void);
void uart_send(unsigned char,unsigned char *);

#endif
=======
#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MAXUARTBUF 4
#if (MAXUARTBUF > MSGLEN)
#define MAXUARTBUF MSGLEN
#endif
typedef struct __uart_comm {
    unsigned char buffer[MAXUARTBUF];
    unsigned char buflen;
} uart_comm;

void init_uart_recv(uart_comm *);
void uart_recv_int_handler(void);
void uart_trans_int_handler(void);
void uart_send(int, unsigned char *);

#endif
>>>>>>> FETCH_HEAD:PIC Motor/src/src/my_uart.h
