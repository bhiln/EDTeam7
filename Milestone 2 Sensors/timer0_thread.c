#include "maindefs.h"
#include <stdio.h>
#include "timer0_thread.h"
#include "messages.h"

// This is a "logical" thread that processes messages from TIMER0
// It is not a "real" thread because there is only the single main thread
// of execution on the PIC because we are not using an RTOS.

int timer0_lthread(timer0_thread_struct *tptr, int msgtype, int length, unsigned char *msgbuffer) {
    unsigned int *msgval;

    SensorData_sendmsg(length, MSGT_SENSOR_DATA, msgbuffer);
    //ToMainHigh_sendmsg(length, MSGT_I2C_DBG, msgbuffer);
    //msgval = (unsigned int *) msgbuffer;
    //unsigned int val = *msgval;
    //SensorData_sendmsg(sizeof(val), MSGT_SENSOR_DATA, (void*) &val);

    //SensorData_recvmsg(MSGLEN, &msgtype, (void *) msgbuffer);
    //unsigned char *msgbuf = msgbuffer;
    //unsigned int* newval = (unsigned int *) msgbuf;
    //*newval is the ADC result in hex format

    // Here is where we would do something with the message
}