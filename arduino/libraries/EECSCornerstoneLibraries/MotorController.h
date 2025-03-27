#ifndef _MOTOR_CONTROLLER_H_
#define _MOTOR_CONTROLLER_H_

#include <ArduinoSTL.h>
#include <utility>

class MotorController {
    private:
    public:
        MotorController();
        MotorController(
            int left_pwm_pin_number, int right_pwm_pin_number,
            int left_first_bridge_pin_number, int left_second_bridge_pin_number, 
            int right_first_bridge_pin_number, int right_second_bridge_pin_number
        );
        void setSpeed(int left_motor_speed, int right_motor_speed); // -255 <= left/right motor speed <= 255
        pair<int, int> currentMotorSpeeds(void);
};

#endif