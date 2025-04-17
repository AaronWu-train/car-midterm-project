#ifndef _CAR_PARAMETERS_H_
#define _CAR_PARAMETERS_H_
typedef unsigned long long ull;

// pins
const int PWMA = 7, AIN1 = 11, AIN2 = 8; // Right motor
const int PWMB = 10, BIN1 = 12, BIN2 = 13; // Left motor
const int LEFT3 = A14, LEFT2 = A8, LEFT1 = A9, MIDDLE = A10, RIGHT1 = A11, RIGHT2 = A12, RIGHT3 = A13; // IR modules
const int RST_PIN = 6, SS_PIN = 53; // RFID
// timing of movements
ull forward_forward_duration = 580;
ull turn_left_forward_duration = 350;
ull turn_right_forward_duration = 350;
ull turn_back_forward_duration = 200;
ull turn_left_duration = 200;
ull turn_right_duration = 350;
ull turn_back_duration = 650;
// motor
double motor_speed_bias = 1 / 1.07; // right motor speed divided by left motor speed
int motor_speed_maximum_difference = 60;
int forward_speed = 200;
int turn_speed = 100;
double turn_speed_ratio = -1.0; // fast wheel speed divided by slow wheel speed
// tracking
double propotional_gain = 0.03;
double ir_weight[7] = {-10.0, -7.0, -5.0, 0.0, 5.0, 7.0, 10.0};
// state extra end function
bool forwardExtraEndCondition(int ir_result[7], int left_speed, int right_speed) {
    return (ir_result[2] && ir_result[3] && ir_result[4] && (ir_result[1] || ir_result[5]));
}
bool turnLeftExtraEndCondition(int ir_result[7], int left_speed, int right_speed) {
    return ir_result[2] || ir_result[3] || ir_result[4]; //ir_result[0] || ir_result[1] || ir_result[2] || ir_result[3];
}
bool turnRightExtraEndCondition(int ir_result[7], int left_speed, int right_speed) {
    return ir_result[2] || ir_result[3] || ir_result[4]; //ir_result[3] || ir_result[4] || ir_result[5] || ir_result[6];
}
bool turnBackExtraEndCondition(int ir_result[7], int left_speed, int right_speed) {
    return ir_result[2] || ir_result[3] || ir_result[4];
}
bool stopExtraEndCondition(int ir_result[7], int left_speed, int right_speed) {
    return !left_speed && !right_speed;
}

#endif