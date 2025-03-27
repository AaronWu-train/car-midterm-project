#ifndef _MOTOR_CONTROLLER_H_
#define _MOTOR_CONTROLLER_H_

class MotorController {
    public: 
        // supposed speed = -255 ~ 255 (negative for backward, not equal to motor RPM)
        double left_motor_speed = 0;
        double right_motor_speed = 0;

        // actual motor speed = motor_speed * motor_bias
        double left_motor_bias = 1.0/1.07;
        double right_motor_bias = 1.0;

        // PIN number for motor control
        int PWMA = 11, AIN2 = 2, AIN1 = 3;  // right motor
        int PWMB = 12, BIN1 = 5, BIN2 = 6;  // left motor

        MotorController(
            int left_pwm_pin_number, int right_pwm_pin_number,
            int left_first_bridge_pin_number, int left_second_bridge_pin_number, 
            int right_first_bridge_pin_number, int right_second_bridge_pin_number
        ) : PWMA(left_pwm_pin_number), PWMB(right_pwm_pin_number), 
            AIN1(right_first_bridge_pin_number), AIN2(right_second_bridge_pin_number),            
            BIN1(left_first_bridge_pin_number), BIN2(left_second_bridge_pin_number){}

        void setBias(int left, int right) {
            left_motor_bias = left;
            right_motor_bias = right;
        }

        void setSpeed(double left_speed, double right_speed) {
            left_motor_speed = left_speed;
            right_motor_speed = right_speed;

            if (right_speed > 0) {
                digitalWrite(AIN1, HIGH);
                digitalWrite(AIN2, LOW);
            } else {
                digitalWrite(AIN1, LOW);
                digitalWrite(AIN2, HIGH);
            }

            if (left_speed > 0) {
                digitalWrite(BIN1, HIGH);
                digitalWrite(BIN2, LOW);
            } else {
                digitalWrite(BIN1, LOW);
                digitalWrite(BIN2, HIGH);
            }
            
            right_speed = min(255, abs(right_speed));
            left_speed  = min(255, abs(left_speed));
        
            analogWrite(PWMA, right_speed * bias);
            analogWrite(PWMB, left_speed);
        }

        void setForward();
        void setBackward();
};

#endif // _MOTOR_CONTROLLER_H_