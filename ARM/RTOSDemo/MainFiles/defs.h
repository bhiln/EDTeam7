#ifndef DEFS_H
#define DEFS_H

/*------------------------------------------------------------------------------
 * Misc. Definitions
 **/

// Joystick pins.
#define JOYSTICK_PORT           1
#define JOYSTICK_LEFT           (1 << 26)
#define JOYSTICK_RIGHT          (1 << 24)
#define JOYSTICK_UP             (1 << 23)
#define JOYSTICK_DOWN           (1 << 25)
#define JOYSTICK_SELECT         (1 << 20)

// Defined data types.
typedef uint8_t bool;
#define true  1
#define false 0

// Defines a command for monitering the stack size to make sure that the stack
// does not overflow. 
#define INSPECT_STACK           1

// Stack sizes.
#define BASE_STACK_SIZE         3
#if PRINTF_VERSION == 1
#define I2C_STACK_SIZE          ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)
#define LCD_STACK_SIZE          ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)
#define CONDUCTOR_STACK_SIZE    ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)
#define SENSORS_STACK_SIZE      ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)
#define LOCATE_STACK_SIZE       ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)
#define COMMAND_STACK_SIZE      ((BASE_STACK_SIZE+5)*configMINIMAL_STACK_SIZE)

#else
#define I2C_STACK_SIZE          (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#define LCD_STACK_SIZE          (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#define CONDUCTOR_STACK_SIZE    (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#define SENSORS_STACK_SIZE      (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#define LOCATE_STACK_SIZE       (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#define COMMAND_STACK_SIZE      (BASE_STACK_SIZE*configMINIMAL_STACK_SIZE)
#endif

// Address of the ARM PIC.
#define SLAVE_ADDR	            0x4F

/*------------------------------------------------------------------------------
 * Message Types
 **/

#define MSG_TYPE_SENSORS        0
#define MSG_TYPE_LOCATE         1
#define MSG_TYPE_CMD	        2
#define MSG_TYPE_ACK            3   
#define MSG_TYPE_LCD_DEBUG      4
#define MSG_TYPE_LCD_SENSORS    5
#define MSG_TYPE_LCD_CMDS       6
#define MSG_TYPE_LCD_ROVER      7

#define MSG_TYPE_TIMER_LCD      10
#define MSG_TYPE_TIMER_SENSORS	11
#define MSG_TYPE_TIMER_LOCATE	12
#define MSG_TYPE_TIMER_CMD	 	13

/*------------------------------------------------------------------------------
 * Rover Definitions
 **/

// Sensors on the rover.
#define SENS_LEN 		    11
#define SENS_IR00           0
#define SENS_IR01           1
#define SENS_IR10           2
#define SENS_IR11           3
#define SENS_IR20           4
#define SENS_IR21           5
#define SENS_IR30           6
#define SENS_IR31           7
#define SENS_IR40           8
#define SENS_IR41           9
#define SENS_AC00           10

// Command message protocol.
#define CMD_LEN             3
#define CMD_TYPE            0
#define CMD_VALUE           1
#define CMD_SPEED           2

#define CMD_STOP            0x00
#define CMD_MF              0x01
#define CMD_MB              0x02
#define CMD_TL              0x03
#define CMD_TR              0x04

// Object protocol.
#define OBJ_LEN             2
#define OBJ_DIST            0
#define OBJ_ANGLE           1

// Orientation protocol.
#define SIDE_LEN            4
#define SIDE_FRONT	        0
#define SIDE_LEFT  	        1
#define SIDE_BACK  	        2
#define SIDE_RIGHT 	        3

// History protocol.
#define HIST_LEN            25

// Corner protocol.
#define CORN_LEN            4

/*------------------------------------------------------------------------------
 * Global Variables
 **/

// Name of the task.
signed char taskNameLCD[]       = "LCD";
signed char taskNameSensors[]   = "Sensor"; 
signed char taskNameLocate[]    = "Locate";
signed char taskNameConductor[] = "Conductor"; 
signed char taskNameCommand[]   = "Command";


#endif
