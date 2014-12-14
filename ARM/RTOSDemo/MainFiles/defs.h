#ifndef DEFS_H
#define DEFS_H


/*------------------------------------------------------------------------------
 * Custom Data Types
 **/

typedef uint8_t bool;
#define true  1
#define false 0

/*------------------------------------------------------------------------------
 * SLAVES
 **/

#define SLAVE_ADDR      0x4F

/*------------------------------------------------------------------------------
 * GPIO
 **/

// LED pins.
#define PORT_LED_0      1
#define PORT_LED_1      1
#define PORT_LED_2      1
#define PORT_LED_3      2
#define PORT_LED_4      2
#define PORT_LED_5      2
#define PORT_LED_6      2
#define PORT_LED_7      2
#define PIN_LED_0       (1 << 28)
#define PIN_LED_1       (1 << 29)
#define PIN_LED_2       (1 << 31)
#define PIN_LED_3       (1 << 2)  
#define PIN_LED_4       (1 << 3)
#define PIN_LED_5       (1 << 4)
#define PIN_LED_6       (1 << 5)
#define PIN_LED_7       (1 << 6)

// Debug pins.
#define PORT_DEBUG      0
#define PIN_DEBUG_0 	(1 << 15)
#define PIN_DEBUG_1 	(1 << 16)
#define PIN_DEBUG_2 	(1 << 17)
#define PIN_DEBUG_3 	(1 << 18)

// Joystick pins.
#define PORT_JOY        1
#define PIN_JOY_LEFT    (1 << 26)
#define PIN_JOY_RIGHT   (1 << 24)
#define PIN_JOY_UP      (1 << 23)
#define PIN_JOY_DOWN    (1 << 25)
#define PIN_JOY_SEL     (1 << 20)

// Pushbutton INT0
#define PORT_PUSH       2
#define PIN_PUSH        (1 << 10)


/*------------------------------------------------------------------------------
 * STACK
 **/

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


/*------------------------------------------------------------------------------
 * Message Types
 **/

#define MSG_TYPE_SENSORS        0
#define MSG_TYPE_LOCATE         1
#define MSG_TYPE_CMD	        2
#define MSG_TYPE_ACK            3   
#define MSG_TYPE_LCD_MOTION     4
#define MSG_TYPE_LCD_GOAL_PRIME 5
#define MSG_TYPE_LCD_GOAL_SEC   6
#define MSG_TYPE_LCD_SIZE_MAP   7
#define MSG_TYPE_LCD_CMD        8
#define MSG_TYPE_LCD_TAB_SENS_0 9
#define MSG_TYPE_LCD_TAB_SENS_1 10
#define MSG_TYPE_LCD_EVENTS     11
#define MSG_TYPE_LCD_SENSORS    12
#define MSG_TYPE_LCD_MAP        13


#define MSG_TYPE_I2C_SENSORS    20
#define MSG_TYPE_I2C_CMD        21
#define MSG_TYPE_I2C_ACK        22

#define MSG_TYPE_TIMER_LCD      30
#define MSG_TYPE_TIMER_SENSORS	31
#define MSG_TYPE_TIMER_LOCATE	32
#define MSG_TYPE_TIMER_CMD	 	33

/*------------------------------------------------------------------------------
 * Rover Definitions
 **/

// Sensors on the rover.
#define SENS_LEN 		    11
#define SENS_RAMP_IR00      0
#define SENS_RAMP_IR01      1
#define SENS_RAMP_AC00      2
#define SENS_OBST_IR10      0
#define SENS_OBST_IR11      1
#define SENS_OBST_IR20      2
#define SENS_OBST_IR21      3
#define SENS_OBST_IR30      4
#define SENS_OBST_IR31      5
#define SENS_OBST_IR40      6
#define SENS_OBST_IR41      7

// Command message protocol.
#define CMD_LEN             7
#define CMD_INIT            0
#define CMD_INDEX           1
#define CMD_MSG_TYPE        2
#define CMD_TYPE            3
#define CMD_VALUE           4
#define CMD_SPEED           5
#define CMD_TERM            6

#define CMD_MF              0x0A
#define CMD_TL              0x0B
#define CMD_TR              0x0C
#define CMD_STOP            0x0D
#define CMD_MB              0x0E

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
signed char taskNameI2C[]       = "I2C task";
signed char taskNameLCD[]       = "LCD Task";
signed char taskNameSensors[]   = "SENSORS task"; 
signed char taskNameLocate[]    = "LOCATE task";
signed char taskNameConductor[] = "CONDUCTOR task"; 
signed char taskNameCommand[]   = "COMMAND task";
signed char taskNameScheduler[] = "SCHEDULER task";

signed char timerNameLCD[]      = "LCD timer";
signed char timerNameLocate[]   = "LOCATE timer";
signed char timerNameCommand[]  = "COMMAND timer";
signed char timerNameSensors[]  = "SENSORS timer";

#endif
