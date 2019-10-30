/**
@file			  MotorControl.c
@author			  JGS
@brief			  Motor control service module related functions definitions
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_err.h"
#include "esp_log.h"

#include "MotorControl.h"
#include "LedControl.h"
#include "IOExtenderControl.h"
#include "OledDisplayManager.h"

#define TAG "MTRCTRL"

//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

#define MTRCTRL_HALL_SENSE_IN		GPIO_NUM_27
#define MTRCTRL_LIMIT_SW_IN			GPIO_NUM_34

#define MTRCTRL_STEP_STEP_OUT	   GPIO_NUM_18
//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions

//_____ P R O T O T Y P E S _________________________________________________
// local function declarations

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
MTRCTRL_MODES mtrctrl_mode = MTRCTRL_MODE_IDLE;
MTRCTRL_EVENTS mtrctrl_event = MTRCTRL_EVENT_NONE;
uint8_t closeHallCounter	 = 0;

static int timecounter=0;
static uint16 fastTimerCtr = 0;

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


/**
  @brief            MotorCtrl_Init
  @details          Initialize Motor control service module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void MotorCtrl_Init(void)
 {
	gpio_pad_select_gpio(MTRCTRL_STEP_STEP_OUT);

	gpio_pad_select_gpio(MTRCTRL_HALL_SENSE_IN);
	gpio_pad_select_gpio(MTRCTRL_LIMIT_SW_IN);

	//Outputs
	gpio_set_direction(MTRCTRL_STEP_STEP_OUT, GPIO_MODE_OUTPUT);

	//Inputs
	gpio_set_direction(MTRCTRL_HALL_SENSE_IN, GPIO_MODE_INPUT);
	gpio_pullup_en(MTRCTRL_HALL_SENSE_IN);
	gpio_set_direction(MTRCTRL_LIMIT_SW_IN, GPIO_MODE_INPUT);

	MotorCtrl_SetEvent(MTRCTRL_EVENT_NONE);
 }

 void MotorCtrl_SetEvent(MTRCTRL_EVENTS event)
 {
	 mtrctrl_event = event;
 }

 int MotorCtrl_isCompletelyOpen(void)
 {
	 return !gpio_get_level(MTRCTRL_LIMIT_SW_IN);
 }

 int MotorCtrl_isCompletelyClosed(void)
 {
	 return !gpio_get_level(MTRCTRL_HALL_SENSE_IN);
 }

 int MotorCtrl_GetSpeed(void)
 {
	 int ret_val = 0;

	 //Not Implemented
	 return ret_val;
 }

 MTRCTRL_DIRECTION MotorCtrl_GetDirection(void)
 {
	 MTRCTRL_DIRECTION ret_val=0;

	 //Not Implemented

	 return ret_val;
 }

 /**
   @brief            MotorCtrl_10msTask
   @details          Periodic 10ms task called by scheduler
   @param[in]        None
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
  void MotorCtrl_10msTask(void)
  {
	  switch(mtrctrl_mode)
	  {
	  	  case MTRCTRL_MODE_IDLE:
	  		  if((mtrctrl_event == MTRCTRL_EVENT_OPEN) || (mtrctrl_event == MTRCTRL_EVENT_CLOSE))
	  		  {
	  			ESP_LOGI(TAG,"Motor Event Received");

	  			IOEC_ControlExternalOutput(extIO_STEPRstPort, extIO_STEPRstPin, 1);
	  			IOEC_ControlExternalOutput(extIO_STEPSleepPort, extIO_STEPSleepPin, 1);

	  			 mtrctrl_mode = MTRCTRL_MODE_STOP_WAIT;
	  			 timecounter=0;
	  		  }
	  		  break;
	  	  case MTRCTRL_MODE_STOP_WAIT:

	  		  timecounter++;
	  		  if(timecounter > MTRCTRL_OFF_WAIT_TIME)
	  		  {

				    timecounter=0;

					if((mtrctrl_event == MTRCTRL_EVENT_OPEN) && (MotorCtrl_isCompletelyOpen() == FALSE))
					{
						ESP_LOGI(TAG,"Motor Open Init");
						MtrCtrl_Event(MTRCTRL_BCAST_EVENT_OPENING_INIT);
						mtrctrl_mode = MTRCTRL_MODE_OPENING;

						IOEC_ControlExternalOutput(extIO_STEPDirPort, extIO_STEPDirPin, 1);
						IOEC_ControlExternalOutput(extIO_STEPEnaPort, extIO_STEPEnaPin, 0);


						LedCtrl_SetMode(LEDCTRL_EVENT_DOOR_MOVING);
						//OLM_SetEvent(OLM_EVENT_DISP_OPENING);

						timecounter=0;
					}
					else if((mtrctrl_event == MTRCTRL_EVENT_CLOSE) && (MotorCtrl_isCompletelyClosed() == FALSE))
					{
						ESP_LOGI(TAG,"Motor Close Init");
						mtrctrl_mode = MTRCTRL_MODE_CLOSING;

						IOEC_ControlExternalOutput(extIO_STEPDirPort, extIO_STEPDirPin, 0);
						IOEC_ControlExternalOutput(extIO_STEPEnaPort, extIO_STEPEnaPin, 0);

						LedCtrl_SetMode(LEDCTRL_EVENT_DOOR_MOVING);
						timecounter=0;
					}
					else
					{
						IOEC_ControlExternalOutput(extIO_STEPEnaPort, extIO_STEPEnaPin, 1);
						IOEC_ControlExternalOutput(extIO_STEPRstPort, extIO_STEPRstPin, 0);
						IOEC_ControlExternalOutput(extIO_STEPSleepPort, extIO_STEPSleepPin, 0);

						//OLM_SetEvent(OLM_EVENT_DISP_OPENING);

						mtrctrl_mode = MTRCTRL_MODE_IDLE;
					}
					mtrctrl_event = MTRCTRL_EVENT_NONE;

	  		  }
	  		  break;
	  	 case MTRCTRL_MODE_LIMIT_SW_WAIT:
			  timecounter++;
			  if(timecounter >= 35)
			  {
				  ESP_LOGI(TAG,"Motor Open Complete");

				mtrctrl_mode = MTRCTRL_MODE_BREAK_WAIT;
				timecounter=0;
				LedCtrl_SetMode(LEDCTRL_EVENT_DOOR_OPEN);
				OLM_SetEvent(OLM_EVENT_DISP_CLOSING);
			  }
			  break;
	  	  case MTRCTRL_MODE_HALL_WAIT:
	  		  timecounter++;
			  if(timecounter >= 10)
			  {
				  ESP_LOGI(TAG,"Motor Close Complete");
					MtrCtrl_Event(MTRCTRL_BCAST_EVENT_DOOR_CLOSED);

					mtrctrl_mode = MTRCTRL_MODE_BREAK_WAIT;
					OLM_SetEvent(OLM_EVENT_DISP_OPENING);
					timecounter=0;
			  }
	  		  break;
	  	  case MTRCTRL_MODE_BREAK_WAIT:

	  		  timecounter++;
	  		  if(timecounter > MTRCTRL_BRAKE_WAIT_TIME)
	  		  {

					timecounter = 0;
					mtrctrl_mode = MTRCTRL_MODE_IDLE;
	  		  }
	  		  break;
	  	  default:
	  		  break;
	  }
  }







  /**
    @brief            MotorCtrl_FastModeTask
    @details          Periodic Fast Mode task called by scheduler
    @param[in]        None
    @param[out]       None
    @param[in,out]    None
    @retval  type     None
  */
   void MotorCtrl_FastModeTask(void)
   {
	   //TODO:  set max number of steps.  If exceeded...blockage probably happened
 	  switch(mtrctrl_mode)
 	  {
 	  	  case MTRCTRL_MODE_OPENING:
 	  		  gpio_set_level(MTRCTRL_STEP_STEP_OUT, 0);
 	  		  vTaskDelay(pdMS_TO_TICKS(1));
 	  		  gpio_set_level(MTRCTRL_STEP_STEP_OUT, 1);


 	  		  if(MotorCtrl_isCompletelyOpen() == TRUE)
 	  		  {
 	  			IOEC_ControlExternalOutput(extIO_STEPEnaPort, extIO_STEPEnaPin, 1);
 	  			IOEC_ControlExternalOutput(extIO_STEPRstPort, extIO_STEPRstPin, 0);
 	  			IOEC_ControlExternalOutput(extIO_STEPSleepPort, extIO_STEPSleepPin, 0);

 	  			mtrctrl_mode = MTRCTRL_MODE_LIMIT_SW_WAIT;
 				timecounter=0;
 				fastTimerCtr = 0;
 	  		  }
 	  		  break;
 	  	  case MTRCTRL_MODE_CLOSING:
 	  	  {
//			bool bRunClosing = false;
// 	  		fastTimerCtr++;
// 	  		if(fastTimerCtr<1020)
// 	  		{
// 	  			bRunClosing = true;
// 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 0);
// 				vTaskDelay(pdMS_TO_TICKS(1));
// 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 1);
//			}
//			else
//			{
//				 if(fastTimerCtr%4 == 0)
//				 {
//					bRunClosing = true;
////		 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 0);
////		 				vTaskDelay(pdMS_TO_TICKS(1));
////		 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 1);
//				 }
//			}

//			if(bRunClosing)
//			{
 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 0);
 				vTaskDelay(pdMS_TO_TICKS(1));
 				gpio_set_level(MTRCTRL_STEP_STEP_OUT, 1);
//			}

 	  		  if(MotorCtrl_isCompletelyClosed() == TRUE)
 	  		  {
 	  			  closeHallCounter++;
 	  		  }
 	  		  else
 	  		  {
 	  			  closeHallCounter = 0;
 	  		  }
 	  		  if(closeHallCounter > 30)
 			  {
 	  			 IOEC_ControlExternalOutput(extIO_STEPEnaPort, extIO_STEPEnaPin, 1);
 	  			 IOEC_ControlExternalOutput(extIO_STEPRstPort, extIO_STEPRstPin, 0);
 	  			 IOEC_ControlExternalOutput(extIO_STEPSleepPort, extIO_STEPSleepPin, 0);

 	  			 mtrctrl_mode = MTRCTRL_MODE_HALL_WAIT;
 				 timecounter=0;
 				 fastTimerCtr = 0;
 				closeHallCounter = 0;
 			  }
 	  		  break;
		  }
 	  	  default:
 	  		  break;
 	  }
   }

 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
