#ifndef _HARDWARE_ACCESS_H_
#define _HARDWARE_ACCESS_H_

#include "HardwareConstants.h"
#include "InfraredModule.h"
#include "MotorController.h"

int PWMA = 11, AIN2 = 3, AIN1 = 2; // right motor
int PWMB = 12, BIN1 = 5, BIN2 = 6; // left motor
MotorController motor_controller(PWMA, PWMB, AIN1, AIN2, BIN1, BIN2);

int IR_MODULE_AMOUNT = 5, IR_INPUT_PINS[5] = {A8, A9, A10, A11, A12};
DigitalInfraredArray digital_infrared_array(IR_MODULE_AMOUNT, &IR_INPUT_PINS[0]);

#endif