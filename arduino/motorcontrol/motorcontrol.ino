#include <Arduino.h>

class MotorController {
   public:
    void init() {
        pinMode(PWMA, OUTPUT);
        pinMode(PWMB, OUTPUT);
        pinMode(AIN1, OUTPUT);
        pinMode(AIN2, OUTPUT);
        pinMode(BIN1, OUTPUT);
        pinMode(BIN2, OUTPUT);
    }

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

    void writeMotorSpeed(double right_speed, double left_speed,
                         double bias = 1.0 / 1.07) {
        right_speed = min(255, max(0, right_speed));
        left_speed  = min(255, max(0, left_speed));

        analogWrite(PWMA, right_speed * bias);
        analogWrite(PWMB, left_speed);
    }

    void setForward() {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        digitalWrite(BIN1, HIGH);
        digitalWrite(BIN2, LOW);
    }

    void setBackward() {
        digitalWrite(AIN1, LOW);
        digitalWrite(AIN2, HIGH);
        digitalWrite(BIN1, LOW);
        digitalWrite(BIN2, HIGH);
    }

    void Tracking(int speed = 50) {
        setForward();

        int l3 = digitalRead(L3);
        int l2 = digitalRead(L2);
        int m  = digitalRead(M);
        int r2 = digitalRead(R2);
        int r3 = digitalRead(R3);
        // if (!l3 && !l2) l2 = 1;
        // if (!r2 && !r3) r2 = 1;


        int w2 = 7;
        int w3 = 10;

        int Kp = speed * 0.2;
        double error = 0;

        if (l3 + l2+m+r2+r3) {
          
        
        error = (double)(l3 * (-w3) + l2 * (-w2) + r2 * w2 + r3 * w3) /
                       (l3 + l2 + m + r2 + r3);
        
        }
        Serial.println(error);
        

        int powerCorrection = Kp * error;

        

        int vR = speed + powerCorrection;  // ex. Kp = 100, 也與w2 & w3有關
        int vL = speed - powerCorrection;  // ex. Tp = 150, 也與w2 & w3有關

        writeMotorSpeed(vL, vR);  // Feedback to motors
    }
};

MotorController motor_controller;


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    motor_controller.init();
}

void loop() {
    // put your main code here, to run repeatedly:
    motor_controller.Tracking();
    delay(10);
}