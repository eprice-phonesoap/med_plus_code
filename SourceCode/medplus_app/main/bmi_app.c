/*
 * bmi_app.c
 *
 *  Created on: Oct 23, 2019
 *      Author: Zap'
 *
 *      Implements necessary features from the BMI270 library files
 */
#include <rom/ets_sys.h>
#include "bmi_app.h"
#include "esp_err.h"
#include "esp_log.h"
#include "I2C_Driver.h"

struct bmi2_dev dev;
struct bmi2_sensor_data sensor_data;
struct bmi2_sens_config config;
uint16_t step_status = 0;
void bmiAccGyroInit(void)
{
//	/* Variable to define rslt */
//	    int8_t rslt;
//	    struct bmi2_dev dev;
//
//	    /*To enable I2C interface*/
//	    dev.read = (bmi2_read_fptr_t)I2C_read_reg;
//	    dev.write = (bmi2_write_fptr_t)I2C_write_reg;
//	    dev.delay_us = delay_us;
//	    dev.read_write_len = 128;
//	    dev.intf = BMI2_I2C_INTERFACE;
//	    dev.dev_id =BMI2_I2C_PRIM_ADDR;
//
//	    dev.config_file_ptr = NULL;
//
//	     /* Initialize BMI2  */
//	    rslt = bmi270_init(&dev);
//	    print_rslt(rslt);

	   // return 0;

	    int8_t rslt;

	    /* Sensor type of sensor to select sensor */
	    uint8_t sensor_sel[2] = { BMI2_ACCEL, BMI2_STEP_COUNTER };

	    /* For status */


	    /* Select features and their pins to be mapped to */
	    struct bmi2_sens_int_config sens_int = { .type = BMI2_STEP_COUNTER, .hw_int_pin = BMI2_INT2 };

	    /* Sensor type of sensor to get data */
	    sensor_data.type = BMI2_STEP_COUNTER;

	    dev.read = (bmi2_read_fptr_t)I2C_read_reg;
	    dev.write = (bmi2_write_fptr_t)I2C_write_reg;
	    dev.delay_us = delay_us;
	    dev.read_write_len = 128;
	    dev.intf = BMI2_I2C_INTERFACE;
	    dev.dev_id = BMI2_I2C_PRIM_ADDR;

	    /* To enable SPI interface*/
	    /*
	     * dev.read = spi_reg_read;
	     * dev.write = spi_reg_write;
	     * dev.delay_us = delay_us;
	     * dev.read_write_len = 4096;
	     * dev.intf = BMI2_SPI_INTERFACE;
	     * dev.dev_id = SPI_CS;
	     * dev.dummy_byte = 1;
	     */

	 	 /* Initialize by enabling firmware download */

	    dev.config_file_ptr = NULL;

	    rslt = bmi270_init(&dev);
	    print_rslt(rslt);

	    /* Enable the accelerometer and step-counter sensor */
	    rslt = bmi2_sensor_enable(sensor_sel, 2, &dev);
	    print_rslt(rslt);

	    /* Update the type of sensor for setting the configurations */
	    config.type = BMI2_STEP_COUNTER;

	    /* Get default configurations for the type of feature selected */
	    rslt = bmi2_get_sensor_config(&config, 1, &dev);
	    print_rslt(rslt);

	    /* Enable water-mark level for to get interrupt after 20 step counts */
	    config.cfg.step_counter.watermark_level = 1;

	    /* Set the configurations */
	    rslt = bmi2_set_sensor_config(&config, 1, &dev);
	    print_rslt(rslt);

	    /* Map the feature interrupt */
	    rslt = bmi2_map_feat_int(&sens_int, 1, &dev);
	    print_rslt(rslt);

	    printf("Move the board for step counter\n");
}

/*!
 *  @brief Function that creates a mandatory delay required in some of the APIs such as "bma4_write_config_file",
 *      "bma4_write_regs", "bma4_set_accel_config"  and so on.
 *
 *  @param[in] period_us  : the required wait time in microseconds.
 *  @return void.
 *
 */


void moveitMoveIt(void)
{
	int8_t rslt;

	rslt = bmi2_get_int_status(&step_status, &dev);
	print_rslt(rslt);

	rslt = bmi2_get_sensor_data(&sensor_data, 1, &dev);
	 print_rslt(rslt);
	 printf("No of steps counted  = %d\n", sensor_data.sens_data.step_counter_output);

//	        if (rslt == BMI2_OK)
//	        {
//	            if (step_status == BMI270_STEP_CNT_STATUS_MASK)
//	            {
//	                printf("Step detected\n");
//
//	                /* Get step counter output */
//	                rslt = bmi2_get_sensor_data(&sensor_data, 1, &dev);
//	                print_rslt(rslt);
//
//	                /* Print the step counter output */
//	                printf("No of steps counted  = %d", sensor_data.sens_data.step_counter_output);
//	                //break;
//	            }
//	        }
	       // dev.delay_us(50000);
}
void enableBMISensor(void)
{
	struct bmi2_dev dev;
	int8_t rslt;
	uint8_t sensor_sel[2] = {BMI2_ACCEL, BMI2_STEP_COUNTER};

	/*To enable I2C interface*/
		    dev.read = (bmi2_read_fptr_t)I2C_read_reg;
		    dev.write = (bmi2_write_fptr_t)I2C_write_reg;
		    dev.delay_us = delay_us;
		    dev.read_write_len = 128;
		    dev.intf = BMI2_I2C_INTERFACE;
		    dev.dev_id = BMI2_I2C_PRIM_ADDR ;

		    dev.config_file_ptr = NULL;

	    rslt = bmi2_sensor_enable(sensor_sel, 2, &dev);
	    print_rslt(rslt);
}
void delay_us(uint32_t period)
{
    /* Wait for a period amount of us*/
	ets_delay_us(period);
}

/*!
 *  @brief Function for writing the sensor's registers through I2C bus.
 *
 *  @param[in] i2c_addr : sensor I2C address.
 *  @param[in] reg_addr : Register address.
 *  @param[in] reg_data : Pointer to the data buffer whose value is to be written.
 *  @param[in] length   : No of bytes to write.
 *
 *  @return Status of execution
 *  @retval 0 -> Success
 *  @retval >0 -> Failure Info
 *
 */

/*!
 *  @brief Prints the execution status of the APIs.
 *
 *  @param[in] api_name : name of the API whose execution status has to be printed.
 *  @param[in] rslt     : error code returned by the API whose execution status has to be printed.
 *
 *  @return void.
 */
void print_rslt(int8_t rslt)
{
    switch (rslt)
    {
        case BMI2_OK:
        	//printf("YOU DID IT!!!!!\n");
            /* Do nothing */
            break;
        case BMI2_E_NULL_PTR:
            printf("Error [%d] : Null pointer\r\n", rslt);
            break;
        case BMI2_E_COM_FAIL:
            printf("Error [%d] : Communication failure\r\n", rslt);
            break;
        case BMI2_E_DEV_NOT_FOUND:
            printf("Error [%d] : Device not found\r\n", rslt);
            break;
        case BMI2_E_INVALID_SENSOR:
            printf("Error [%d] : Invalid sensor\r\n", rslt);
            break;
        case BMI2_E_SELF_TEST_FAIL:
            printf("Warning [%d] : Self test failed\r\n", rslt);
            break;
        case BMI2_E_INVALID_INT_PIN:
            printf("warning [%d] : invalid int pin\r\n", rslt);
            break;
        default:
            printf("Error [%d] : Unknown error code\r\n", rslt);
            break;
    }
}
