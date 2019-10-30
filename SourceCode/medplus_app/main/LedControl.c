/**
@file			  LedControl.c
@author			  JGS
@brief			  LED control service module related functions definitions

@copyright		  PSDI

@attention		  The information contained herein is confidential property of
                  PSDI. The use, copy, transfer or disclosure
                  of such information is prohibited except by express written
                  agreement with Directed Electronics Inc.
*/


//_____I N C L U D E ________________________________________________________
// Place here all includes needed by this file starting with its own header
// showing the reason of the include.
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "LedControl.h"
#include "math.h"
#include "esp_log.h"


//_____D E F I N E __________________________________________________________
// local private defines, enums, macros
#define LEDCTRL_RED_PORT		GPIO_NUM_19
#define LEDCTRL_GREEN_PORT		GPIO_NUM_21
#define LEDCTRL_BLUE_PORT		GPIO_NUM_22


#define LEDCTRL_RED_PWM_CHANNEL  LEDC_CHANNEL_0
#define LEDCTRL_GREEN_PWM_CHANNEL  LEDC_CHANNEL_1
#define LEDCTRL_BLUE_PWM_CHANNEL  LEDC_CHANNEL_2

#define LEDCTRL_PWM_TIMER LEDC_TIMER_2
#define LEDCTRL_PWM_BIT_NUM LEDC_TIMER_13_BIT

#define LEDCTRL_MAX_DUTY ((1 << LEDCTRL_PWM_BIT_NUM) - 1)
#define LEDCTRL_DUTY(x)  ((x * LEDCTRL_MAX_DUTY) / 255)

//_____ T Y P E D E F _______________________________________________________
// local private typedefs, structs, unions

typedef enum
{
	LEDCTRL_MODE_NONE=0, //No LED activity
	LEDCTRL_MODE_SELF_TEST,
	LEDCTRL_MODE_WIFI_CREDENTIALS_OK, //Blue blink 2Hz
	LEDCTRL_MODE_WIFI_NOT_OK, //Red blink 2Hz
	LEDCTRL_MODE_WIFI_CONNECTED, //Green blink 3 times in 1 second
	LEDCTRL_MODE_ENTERING_SETUP, //Blink green/blue alternatively 2Hz
	LEDCTRL_MODE_CLEANING_IN_PROGRESS,
	LEDCTRL_MODE_CLEANING_NEARING_COMPLETION,
	LEDCTRL_MODE_CLEANING_COMPLETE,
	LEDCTRL_MODE_CLEANING_ABORTED,
	LEDCTRL_MODE_DOOR_MOVING,
	LEDCTRL_MODE_DOOR_OPEN,
}LEDCTRL_MODES;

unsigned char Red;
unsigned char Green;
unsigned char Blue;

float Hue;
float Saturation;
float Value;

//_____ P R O T O T Y P E S _________________________________________________
// local function declarations
void LedCtrl_HandleEvents(void);
void LedCtrl_SetValues(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness);
uint8_t LedCtrl_ConvertValues(uint8_t value);

static double Min(double a, double b);
static double Max(double a, double b);

void HSVToRGB(float hue, float saturation, float value);
void RGBToHSV(uint8_t red, uint8_t green, uint8_t blue);

//_____ L O C A L  D A T A __________________________________________________
// local private data (static)
LEDCTRL_MODES ledctrl_mode;
LEDCTRL_EVENTS ledctrl_event;
static int led_timecounter = 0;
static int blink_times=0;
//_____ P U B L I C   F U N C T I O N S _____________________________________
// public interface functions


/**
  @brief            LedCtrl_Init
  @details          Initialize LED control service module
  @param[in]        None
  @param[out]       None
  @param[in,out]    None
  @retval  type     None
*/
 void LedCtrl_Init(void)
 {

	ledc_channel_config_t ledc_channel_red = {0};
	ledc_channel_config_t ledc_channel_blue = {0};
	ledc_channel_config_t ledc_channel_green = {0};


	ledc_channel_red.gpio_num = LEDCTRL_RED_PORT;
	ledc_channel_red.speed_mode = LEDC_LOW_SPEED_MODE;
	ledc_channel_red.channel = LEDCTRL_RED_PWM_CHANNEL;
	ledc_channel_red.intr_type = LEDC_INTR_DISABLE;
	ledc_channel_red.timer_sel = LEDCTRL_PWM_TIMER;
	ledc_channel_red.duty = 0;

	ledc_channel_blue.gpio_num = LEDCTRL_BLUE_PORT;
	ledc_channel_blue.speed_mode = LEDC_LOW_SPEED_MODE;
	ledc_channel_blue.channel = LEDCTRL_BLUE_PWM_CHANNEL;
	ledc_channel_blue.intr_type = LEDC_INTR_DISABLE;
	ledc_channel_blue.timer_sel = LEDCTRL_PWM_TIMER;
	ledc_channel_blue.duty = 0;

	ledc_channel_green.gpio_num = LEDCTRL_GREEN_PORT;
	ledc_channel_green.speed_mode = LEDC_LOW_SPEED_MODE;
	ledc_channel_green.channel = LEDCTRL_GREEN_PWM_CHANNEL;
	ledc_channel_green.intr_type = LEDC_INTR_DISABLE;
	ledc_channel_green.timer_sel = LEDCTRL_PWM_TIMER;
	ledc_channel_green.duty = 0;

	ledc_timer_config_t status_led_timer = {0};

	status_led_timer.speed_mode = LEDC_LOW_SPEED_MODE;
	status_led_timer.bit_num = LEDCTRL_PWM_BIT_NUM;
	status_led_timer.timer_num = LEDCTRL_PWM_TIMER;
	status_led_timer.freq_hz = 5000;

	ESP_ERROR_CHECK( ledc_channel_config(&ledc_channel_red) );
	ESP_ERROR_CHECK( ledc_channel_config(&ledc_channel_blue) );
	ESP_ERROR_CHECK( ledc_channel_config(&ledc_channel_green) );

	ESP_ERROR_CHECK( ledc_timer_config(&status_led_timer) );

	LedCtrl_SetValues(0x00,0x00,0x00,0);
	LedCtrl_SetMode(LEDCTRL_MODE_SELF_TEST);
 }

 /**
   @brief            LedCtrl_SetMode
   @details          Set mode of LED Status
   @param[in]        mode - LED Mode
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
 void LedCtrl_SetMode(LEDCTRL_EVENTS event)
 {
	 ledctrl_event = event;

 }

 /**
   @brief            LedCtrl_10msTask
   @details          Periodic 10ms task to be called by Scheduler
   @param[in]        None
   @param[out]       None
   @param[in,out]    None
   @retval  type     None
 */
void LedCtrl_10msTask(void)
{
	LedCtrl_HandleEvents();

	//BLUFI_INFO("led_timecounter = %d, state = %d, Level = %d", led_timecounter, ledctrl_mode, gpio_get_level(LEDCTRL_RED_PORT));
	switch(ledctrl_mode)
	{
		case LEDCTRL_MODE_NONE:

			break;
		case LEDCTRL_MODE_SELF_TEST:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0xFF,0xFF,0x00,100);
			}
			if(led_timecounter == MILLI_SECONDS_IN_10MS_LOOP(200))
			{

				LedCtrl_SetValues(0x00,0xFF,0xFF,100);
			}
			if(led_timecounter == MILLI_SECONDS_IN_10MS_LOOP(400))
			{
				LedCtrl_SetValues(0x00,0x00,0xFF,100);
			}
			led_timecounter++;
			if(led_timecounter >= MILLI_SECONDS_IN_10MS_LOOP(600))
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_ENTERING_SETUP: //Blue blink 2Hz
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0x00,0x00,0xFF,50);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_WIFI_NOT_OK:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0xFF,0x00,0x00,50);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_WIFI_CONNECTED:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0x00,0xFF,0x00,50);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_WIFI_CREDENTIALS_OK:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0xFF,0x00,50);
			}
			if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0x00,0x00,0xFF,50);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_CLEANING_IN_PROGRESS:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			else if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0x00,0x00,0xFF,100);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_CLEANING_NEARING_COMPLETION:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			else if(led_timecounter == 25)
			{
				LedCtrl_SetValues(0x00,0x00,0xFF,100);
			}
			led_timecounter++;
			if(led_timecounter >= 50)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_CLEANING_COMPLETE:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0xFF,0x00,100);
			}

			led_timecounter++;
			if(led_timecounter >= 1000)
			{
				led_timecounter = 0;
				ledctrl_mode = LEDCTRL_MODE_NONE;
				LedCtrl_SetMode(LEDCTRL_EVENT_NONE);
			}
			break;
		case LEDCTRL_MODE_CLEANING_ABORTED:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			else if(led_timecounter == 25)
			{
				LedCtrl_SetValues(0xFF,0x00,0x00,100);
			}
			led_timecounter++;
			if(led_timecounter >= 50)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_DOOR_MOVING:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			else if(led_timecounter == 25)
			{
				LedCtrl_SetValues(0x00,0xFF,0x00,100);
			}
			led_timecounter++;
			if(led_timecounter >= 50)
			{
				led_timecounter = 0;
			}
			break;
		case LEDCTRL_MODE_DOOR_OPEN:
			if(led_timecounter == 0)
			{
				LedCtrl_SetValues(0x00,0x00,0x00,0);
			}
			else if(led_timecounter == 50)
			{
				LedCtrl_SetValues(0x00,0xFF,0x00,100);
			}
			led_timecounter++;
			if(led_timecounter >= 100)
			{
				led_timecounter = 0;
			}
			break;
		default:
			break;
	}


}


 //_____ L O C A L  F U N C T I O N S ________________________________________
 // local interface functions
void LedCtrl_HandleEvents(void)
{
	if(ledctrl_event != LEDCTRL_EVENT_NONE)
	{
		//BLUFI_INFO("LED Event Received : %d", ledctrl_event);
		led_timecounter = 0;
		blink_times=0;
		LedCtrl_SetValues(0x00,0x00,0x00,0);
		ledctrl_mode = LEDCTRL_MODE_NONE;

	}
	if(ledctrl_event == LEDCTRL_EVENT_SELF_TEST)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_SELF_TEST;
	}
	if(ledctrl_event == LEDCTRL_EVENT_WIFI_CREDENTIALS_OK)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_WIFI_CREDENTIALS_OK;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_WIFI_NOT_OK)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_WIFI_NOT_OK;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_WIFI_CONNECTED)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_WIFI_CONNECTED;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_ENTERING_SETUP)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_ENTERING_SETUP;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_ALL_OFF)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		LedCtrl_SetValues(0x00,0x00,0x00,0);
		ledctrl_mode = LEDCTRL_MODE_NONE;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_CLEANING_IN_PROGRESS)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_CLEANING_IN_PROGRESS;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_CLEANING_NEARING_COMPLETION)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_CLEANING_NEARING_COMPLETION;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_CLEANING_COMPLETE)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_CLEANING_COMPLETE;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_CLEANING_ABORTED)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_CLEANING_ABORTED;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_DOOR_MOVING)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_DOOR_MOVING;
	}
	else if(ledctrl_event == LEDCTRL_EVENT_DOOR_OPEN)
	{
		ledctrl_event = LEDCTRL_EVENT_NONE;
		ledctrl_mode = LEDCTRL_MODE_DOOR_OPEN;
	}

}

uint8_t LedCtrl_ConvertValues(uint8_t value)
{
	int16_t y=0;
	y=((int16_t)-1 * (int16_t)value )+(int16_t)255;
	return (uint8_t)y;
}

void LedCtrl_SetValues(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness)
 {

	 RGBToHSV(red, green, blue);
	 HSVToRGB(Hue,Saturation,((Value*brightness)/100)); //Hue, Saturation,Value are global variables
	                                                    //as a result of RGB to HSV Conversion
	                                                    //Global values used to save stack space

	 //printf("Input RGBA=%x,%x,%x,%d : HSV=%f,%f,%f : Output RGB=%x,%x,%x\n",red, blue, green, brightness, Hue, Saturation, Value, Red, Green, Blue);
	 ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_RED_PWM_CHANNEL, LEDCTRL_DUTY(LedCtrl_ConvertValues(Red)));
	 ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_RED_PWM_CHANNEL);

	 ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_GREEN_PWM_CHANNEL, LEDCTRL_DUTY(LedCtrl_ConvertValues(Green)));
	 ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_GREEN_PWM_CHANNEL);

	 ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_BLUE_PWM_CHANNEL, LEDCTRL_DUTY(LedCtrl_ConvertValues(Blue)));
	 ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDCTRL_BLUE_PWM_CHANNEL);

 }



static double Min(double a, double b) {
	return a <= b ? a : b;
}

static double Max(double a, double b) {
	return a >= b ? a : b;
}

void RGBToHSV(uint8_t red, uint8_t green, uint8_t blue) {
	float delta, min;
	float h = 0, s, v;

	min = Min(Min(red, green), blue);
	v = Max(Max(red, green), blue);
	delta = v - min;

	if (v == 0.0)
		s = 0;
	else
		s = delta / v;

	if (s == 0)
		h = 0.0;

	else
	{
		if (red == v)
			h = (green - blue) / delta;
		else if (green == v)
			h = 2 + (blue - red) / delta;
		else if (blue == v)
			h = 4 + (red - green) / delta;

		h *= 60;

		if (h < 0.0)
			h = h + 360;
	}
	Hue = h;
	Saturation = s;
	Value = v / 255;

	return;
}

void HSVToRGB(float hue, float saturation, float value) {
	float r = 0, g = 0, b = 0;

	if (saturation == 0)
	{
		r = value;
		g = value;
		b = value;
	}
	else
	{
		int i;
		float f, p, q, t;

		if (hue == 360)
			hue = 0;
		else
			hue = hue / 60;

		i = (int)trunc(hue);
		f = hue - i;

		p = value * (1.0 - saturation);
		q = value * (1.0 - (saturation * f));
		t = value * (1.0 - (saturation * (1.0 - f)));

		switch (i)
		{
		case 0:
			r = value;
			g = t;
			b = p;
			break;

		case 1:
			r = q;
			g = value;
			b = p;
			break;

		case 2:
			r = p;
			g = value;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = value;
			break;

		case 4:
			r = t;
			g = p;
			b = value;
			break;

		default:
			r = value;
			g = p;
			b = q;
			break;
		}

	}


	Red = r * 255;
	Green = g * 255;
	Blue = b * 255;

	return;
}

