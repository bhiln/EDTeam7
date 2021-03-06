#ifndef __my_uart_h
#define __my_uart_h

#include "messages.h"

#define MAXUARTBUF 26
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
    unsigned char msg_count;
    unsigned char cmd_count;
} uart_comm;

void init_uart_recv(uart_comm *);
void uart_recv_int_handler(void);
void uart_send(unsigned char,unsigned char *);
void uart_trans_int_handler(void);

#endif
