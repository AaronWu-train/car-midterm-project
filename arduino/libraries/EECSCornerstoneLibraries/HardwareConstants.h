#ifndef _HARDWARE_CONSTANTS_H_
#define _HARDWARE_CONSTANTS_H_

// Right motor
int PWMA = 11;
int AIN2 = 3;
int AIN1 = 2;

// Left motor
int PWMB = 12;
int BIN1 = 5;
int BIN2 = 6;

// Tracking sensor
int L3 = A8;
int L2 = A9;
int M  = A10;
int R2 = A11;
int R3 = A12;

constexpr double SPEED_PROPORTION = 1 / 1.07; // right speed * proportion == left speed

#endif