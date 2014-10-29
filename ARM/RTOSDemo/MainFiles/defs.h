#ifndef DEFS_H
#define DEFS_H

// Defined data types.
typedef uint8_t bool;
#define true  1
#define false 0

// Maximum length of a message that can be received by the task.
#define maxLenSensors (sizeof(portTickType))
#define maxLenLocate  (sizeof(portTickType))
#define maxLenLCD     20

// Name of the task.
signed char taskNameSensors[]    = "Sensor"; 
signed char taskNameLocate[]    = "Locate";
signed char taskNameConductor[] = "Conductor"; 

// Number of sensors on the rover.
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

// Maximum number of commands that are saved in history list.
#define CMD_HIST_LEN        100
#define SM_HIST_LEN         100
#define SPG_HIST_LEN        100 
#define SSG_HIST_LEN        100

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

// Orientation definitions.
#define SIDE_LEN            4
#define SIDE_FRONT	        0
#define SIDE_RIGHT 	        1
#define SIDE_BACK  	        2
#define SIDE_LEFT  	        3

#endif
