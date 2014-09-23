#ifndef I2CTASK_MSG_TYPES_H
#define I2CTASK_MSG_TYPES_H

// Here is where I define the types of the messages that I am passing to the I2C task
//   --Note that none of these message types (as I have implemented this) actually go over the I2C bus, but they
//     are useful for matching up what is send to/from the I2C task message queues
//
// I have defined them all here so that they are unique

#define vtI2CMsgTypeIR0Init 1
#define vtI2CMsgTypeIR0ReadByte0 2
#define vtI2CMsgTypeIR0ReadByte1 3

#define IR0MsgTypeTimer 5 
#endif