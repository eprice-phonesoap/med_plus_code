/*
 * bmi_app.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Zap
 */

#ifndef MAIN_BMI_APP_H_
#define MAIN_BMI_APP_H_

#include <stdio.h>
#include "bmi2.h"
#include "bmi270.h"

void delay_us(uint32_t period);

void print_rslt(int8_t rslt);

void bmiAccGyroInit(void);
void enableBMISensor(void);
void moveitMoveIt(void);
#endif /* MAIN_BMI_APP_H_ */