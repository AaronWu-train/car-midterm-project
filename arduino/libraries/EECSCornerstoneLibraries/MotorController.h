#ifndef _MOTOR_CONTROLLER_H_
#define _MOTOR_CONTROLLER_H_

struct PulseWidthModulationController {
    int m_pin_number, m_current_speed;
    PulseWidthModulationController(void);
    PulseWidthModulationController(int pin_number);
    void init(int pin_number);
    void output(int value); // 0 <= value <= 255
    int currentSpeed(void); // used to check if current speed is same as previous in order not to repeatedly output the same value
};

struct BridgeCircuitDirectionController {
    int m_first_pin_number, m_second_pin_number;
    bool m_current_direction;
    BridgeCircuitDirectionController(void);
    BridgeCircuitDirectionController(int first_pin_number, int second_pin_number);
    void init(int first_pin_number, int second_pin_number);
    void output(bool direction); // 0 for forward, 1 for backward
    bool currentDirection(void); // used to check if current direction is same as previous in order not to repeatedly output the same value
};

class MotorController {
    private:
        PulseWidthModulationController m_left_pwm_controller, m_right_pwm_controller;
        BridgeCircuitDirectionController m_left_bridge_controller, m_right_bridge_controller;
    public:
        MotorController();
        MotorController(
            int left_pwm_pin_number, int right_pwm_pin_number,
            int left_first_bridge_pin_number, int left_second_bridge_pin_number, 
            int right_first_bridge_pin_number, int right_second_bridge_pin_number
        );
        void setSpeed(int left_motor_speed, int right_motor_speed); // -255 <= left/right motor speed <= 255
};

#endif