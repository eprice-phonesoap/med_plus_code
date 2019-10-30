/**
@file			  MotorControl.h
@author			  JGS
@brief			  Motor Control Interface related structure definitions and function prototype declaration
*/

#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_

//_____I N C L U D E ________________________________________________________
// mandatory includes needed by this file (hard dependencies)
#include "Std_Types.h"

//_____D E F I N E __________________________________________________________
// public defines, enums and macros
#define MTRCTRL_DOOR_CLOSE_SPEED 60
#define MTRCTRL_DOOR_OPEN_SPEED 50

#define MTRCTRL_OFF_WAIT_TIME 10
#define MTRCTRL_BRAKE_WAIT_TIME 50
//_____ T Y P E D E F _______________________________________________________
// public typedefs, structs, unions
typedef enum
{
	MTRCTRL_MODE_IDLE=0,
	MTRCTRL_MODE_STOP_WAIT,
	MTRCTRL_MODE_OPENING,
	MTRCTRL_MODE_CLOSING,
	MTRCTRL_MODE_LIMIT_SW_WAIT,
	MTRCTRL_MODE_HALL_WAIT,
	MTRCTRL_MODE_BREAK_WAIT
}MTRCTRL_MODES;

typedef enum
{
	MTRCTRL_EVENT_NONE=0,
	MTRCTRL_EVENT_OPEN,
	MTRCTRL_EVENT_CLOSE,
	MTRCTRL_EVENT_STOP,
}MTRCTRL_EVENTS;

typedef enum
{
	MTRCTRL_BCAST_EVENT_NONE=0,
	MTRCTRL_BCAST_EVENT_DOOR_CLOSED,
	MTRCTRL_BCAST_EVENT_OPENING_INIT,
}MTRCTRL_BCAST_EVENTS;

typedef enum
{
	MTRCTRL_FORWARD=0,
	MTRCTRL_REVERSE=1
}MTRCTRL_DIRECTION;

//_____ P R O T O T Y P E S _________________________________________________
// public interface (API)
void MotorCtrl_Init(void);
void MotorCtrl_SetEvent(MTRCTRL_EVENTS event);
int MotorCtrl_GetSpeed(void);
MTRCTRL_DIRECTION MotorCtrl_GetDirection(void);
int MotorCtrl_isCompletelyOpen(void);
int MotorCtrl_isCompletelyClosed(void);
void MotorCtrl_10msTask(void);
void MotorCtrl_FastModeTask(void);

extern void MtrCtrl_Event(MTRCTRL_BCAST_EVENTS mtrctrl_event);

//_____ P R O T O T Y P E S _________________________________________________
// local interface (API)

#endif /* MOTORCONTROL_H_ */
