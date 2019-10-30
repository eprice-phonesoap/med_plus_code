/**
@file			  UVCleaningApp.c
@author			  JGS
@brief			  UV Cleaning Application related functions definitions
*/

//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ProximitySense.h"
#include "MotorControl.h"
#include "I2C_Driver.h"
#include "BeepControl.h"

#define TAG "PROX_SENSE"
//_____D E F I N E __________________________________________________________
// local private defines, enums, macros

//I2C info
#define i2cAddrProx 0x24

//Registers
#define reg_PID			0x00
#define reg_CONFIG		0x01
#define reg_INTERRUPT	0x02
#define reg_PS_LT		0x03
#define reg_PS_HT		0x04
#define reg_ALSIR_TH1	0x05
#define reg_ALSIR_TH2	0x06
#define reg_ALSIR_TH3	0x07
#define reg_PS_DATA		0x08
#define reg_ALSIR_DT1	0x09
#define reg_ALSIR_DT2	0x0A
#define reg_RESET		0x0E
#define reg_OFFSET		0x0F

//Bits
#define ALSIR_MODE 0
#define ALS_RANGE 1
#define ALS_EN 2
#define PS_DR0 3
#define PS_DR1 4
#define PS_DR2 5
#define PS_SLP 6
#define PS_EN 7

#define KAL_TRUE	1
#define KAL_FALSE	0


//#define PROX_DETECT_THRESHOLD 150
#define PROX_DETECT_THRESHOLD 99 // 4inch


#define BTNCTRL_DEBOUNCE_FREQ 10	// Read hardware every 10 msec
#define BTNCTRL_PRESS_MSEC 20		// Stable time in msec before registering pressed
#define BTNCTRL_RELEASE_MSEC 10	// Stable time in msec before registering released



//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions


//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
uint8_t HRS_ReadBytes(uint8_t* Data, uint8_t RegAddr);
uint8_t HRS_WriteBytes(uint8_t RegAddr, uint8_t Data);

void BtnCtrl_DebouncePowerSwitch(boolean *Key_pressed);
void HRS_DelayMS(uint16_t delay);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)

uint8_t ProxDetectedFlag = FALSE;
uint8_t ProxSensorLevel = 0;

BtnState_Type PowerBtnState = BTN_STATE_OFF;
boolean PowerBtnStateChanged = FALSE;

static boolean DebouncedKeyPress = FALSE;
static uint8_t Count = BTNCTRL_RELEASE_MSEC / BTNCTRL_DEBOUNCE_FREQ;

uint16 btnPowerBtnPressCounter = 0;
uint16 btnPowerBtnReleaseCounter = 0;

//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions

void ProximitySense_Init(void)
{
	uint8_t confRegTemp = 0x00;

	//Trigger soft reset, data defined by datasheet
	HRS_WriteBytes(reg_RESET, 0x9c);
	HRS_WriteBytes(reg_OFFSET, 0xe1);
	HRS_DelayMS(300);

	//Exit reset to normal operation
	HRS_WriteBytes(reg_RESET, 0x00);
	HRS_WriteBytes(reg_OFFSET, 0x00);


	HRS_WriteBytes(reg_PS_LT, 0x00);
	HRS_WriteBytes(reg_PS_HT, 0x00);

	confRegTemp |= ((1 << ALSIR_MODE) | (1 << ALS_RANGE) | (1 << ALS_EN) | (0 << PS_DR0) | (1 << PS_DR1) | (1 << PS_DR2) | (0 << PS_SLP) | (1 << PS_EN));
	HRS_WriteBytes(reg_CONFIG, confRegTemp);
}

uint8_t ProximitySense_ReadLevel(void)
{
	uint8_t senseDataOut = 0;
	//Check proximity detection level from sensor
	HRS_ReadBytes(&senseDataOut, reg_PS_DATA);
	return senseDataOut;
}

void ProximitySense_1msTasks(void)
{

	uint8 KeyPressed = FALSE;

	BtnCtrl_DebouncePowerSwitch(&KeyPressed); /**Get touch state and perform debounce on it*/

	if(KeyPressed == FALSE)
	{
		//Button events are processed after touch is released
		btnPowerBtnPressCounter = 0;
		if(btnPowerBtnReleaseCounter == 10)
		{
			PowerBtnStateChanged = FALSE;
			//BtnCtrl_Event(BTN_STATE_OFF);  /**Send touch event with OFF status*/
			ESP_LOGI(TAG,"Button OFF Detected");
		}
		if(btnPowerBtnReleaseCounter < 60)
		{
			btnPowerBtnReleaseCounter++;
		}
	}
	else
	{
		/**Increment button press counter until it is released*/
		btnPowerBtnReleaseCounter = 0;
		PowerBtnStateChanged = FALSE;
		if(btnPowerBtnPressCounter < 60)
		{
			btnPowerBtnPressCounter++;
		}
		if(btnPowerBtnPressCounter == 20)
		{
			bool runMtrControlEvent = false;

			ESP_LOGI(TAG,"Button ON Detected");
			BeepControl_SetEvent(BEEP_START);

			if(MotorCtrl_isCompletelyOpen() == TRUE)
			{
				MotorCtrl_SetEvent(MTRCTRL_EVENT_CLOSE);
				ESP_LOGI(TAG,"Motor Close Event");
				BeepControl_SetEvent(BEEP_START);
			}
			else if(MotorCtrl_isCompletelyClosed() == TRUE)
			{
				runMtrControlEvent = true;
//				MotorCtrl_SetEvent(MTRCTRL_EVENT_OPEN);
//				ESP_LOGI(TAG,"Motor Open Event");
			}
			else
			{
				runMtrControlEvent = true;
//				MotorCtrl_SetEvent(MTRCTRL_EVENT_OPEN);
//				ESP_LOGI(TAG,"Motor Open Event");
			}

			if(runMtrControlEvent)
			{
				MotorCtrl_SetEvent(MTRCTRL_EVENT_OPEN);
				ESP_LOGI(TAG,"Motor Open Event");
				BeepControl_SetEvent(BEEP_START);
			}
			btnPowerBtnPressCounter = 60;
		}
	}
}
 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions

uint8_t HRS_ReadBytes(uint8_t* Data, uint8_t RegAddr)
{

	//Select register to read
	I2C_write_byte(i2cAddrProx, RegAddr);

	//Read register
	I2C_read(i2cAddrProx, Data, 1);

	return KAL_TRUE;
}

uint8_t HRS_WriteBytes(uint8_t RegAddr, uint8_t Data)
{

	uint8_t i2cArrayOut[2] = {RegAddr, Data};
	I2C_write(i2cAddrProx, i2cArrayOut, 2);
	return KAL_TRUE;
}

void HRS_DelayMS(uint16_t delay)
{
	vTaskDelay(pdMS_TO_TICKS(delay));
}

void BtnCtrl_DebouncePowerSwitch(uint8 * Key_pressed)
{

	 /** Measure touch sensor sense from PTC*/
	 *Key_pressed = DebouncedKeyPress;

	 ProxSensorLevel = ProximitySense_ReadLevel();
	 //ESP_LOGI(TAG,"Prox Level %d", ProxSensorLevel);

	  if(ProxSensorLevel > PROX_DETECT_THRESHOLD)
	  {
		  ProxDetectedFlag = TRUE;
	  }
	  else
	  {
		  ProxDetectedFlag = FALSE;
	  }

	 /**Perform debounce on the touch button state */
	 if (ProxDetectedFlag == DebouncedKeyPress)
	 {
		 // Set the timer which allows a change from current state.
		 if (DebouncedKeyPress)
			Count = BTNCTRL_RELEASE_MSEC / BTNCTRL_DEBOUNCE_FREQ;
		 else
			Count = BTNCTRL_PRESS_MSEC / BTNCTRL_DEBOUNCE_FREQ;
	}
	else
	{
		 // Key has changed - wait for new state to become stable.
		 if (--Count == 0)
		 {
			 // Timer expired - accept the change.
			 DebouncedKeyPress = ProxDetectedFlag;
			 *Key_pressed=DebouncedKeyPress;
			 // And reset the timer.
			 if (DebouncedKeyPress)
			 {
				Count = BTNCTRL_RELEASE_MSEC / BTNCTRL_DEBOUNCE_FREQ;
			 }
			 else
			 {
				Count = BTNCTRL_PRESS_MSEC / BTNCTRL_DEBOUNCE_FREQ;
			 }
		 }
	 }
}

