#include "HardwareConstants.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  //SCL->A5, SDA->A4, VCC->5V
int state = 0, out = 0; // 0 for forward, 1 for turning right, 2 for waiting for all white

char line1[16] = "state:          ";
char line2[16] = "out  :          ";

void writeMotorSpeed(double right_speed, double left_speed, double bias = 1.0 / 1.07) {
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

void tracking(int speed = 55) {
    int l3 = digitalRead(L3);
    int l2 = digitalRead(L2);
    int m  = digitalRead(M);
    int r2 = digitalRead(R2);
    int r3 = digitalRead(R3);
    // if (!l3 && !l2) l2 = 1;
    // if (!r2 && !r3) r2 = 1;
    int tot = l3 + l2 + m + r2 + r3;
    if (state == 0) {
      if (tot >= 4) {
        // meet all black
        if (out == 1) {
          state = 2;
        } else {
          state = 1;
        }
      }
    } else if (state == 1) {
      if ((m || l2 || r2) && tot <= 2 && (!r3 && !l3)) {
        state = 0;
        out = 1 - out;
      }
    } else {
      if (tot == 0) {
        state = 1;
      }
    }
    int vR, vL;
    if (state == 0) {
        int w2 = 5;
        int w3 = 7;
        double Kp = 0.05 * speed;
        double error = 0;
        if (l3 + l2+m+r2+r3) {    
            error = (double)(l3 * (-w3) + l2 * (-w2) + r2 * w2 + r3 * w3) / (l3 + l2 + m + r2 + r3);
        }
        int powerCorrection = Kp * error;
        vR = speed + powerCorrection;  // ex. Kp = 100, 也與w2 & w3有關
        vL = speed - powerCorrection;  // ex. Tp = 150, 也與w2 & w3有關
    } else if (state == 1) {
        if (out == 0) {
            vR = speed;
            vL = speed * 0.4;
        } else {
            vR = 0.8 * speed;
            vL = -0.8 * speed;
        }
    } else {
        vR = vL = speed;
    }
    line1[6] = 48 + state;
    line2[6] = 48 + out;
    Serial.println(vR);
    Serial.println(vL);
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    writeMotorSpeed(vL, vR);  // Feedback to motors
}

void setup() {  
    pinMode(PWMA, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    delay(1000);
}

void loop() {
    tracking();
    delay(2);
}