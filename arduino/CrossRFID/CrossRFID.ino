#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  //SCL->A5, SDA->A4, VCC->5V
char line1[16] = "counter:        ";
char line2[16] = "                ";

const int PWMA = 9, AIN1 = 11, AIN2 = 8; // Right motor
const int PWMB = 10, BIN1 = 12, BIN2 = 13; // Left motor
const int LEFT2 = A8, LEFT1 = A9, MIDDLE = A10, RIGHT1 = A11, RIGHT2 = A12; // IR modules

struct DigitalIR {
    int pin;
    DigitalIR() {}
    DigitalIR(int pin) : pin(pin) { pinMode(pin, INPUT); }
    int read() { return digitalRead(pin); }
};
struct Motor {
    int pwm_pin, dir_pin1, dir_pin2; // motor controll pins
    int now_dir, now_speed;
    int speed_delta = 20;
    Motor() {}
    Motor(int pwm_pin, int dir_pin1, int dir_pin2) : pwm_pin(pwm_pin), dir_pin1(dir_pin1), dir_pin2(dir_pin2), now_dir(-1), now_speed(0) {
        pinMode(pwm_pin, OUTPUT);
        pinMode(dir_pin1, OUTPUT);
        pinMode(dir_pin2, OUTPUT);
    }
    void setSpeed(int speed) {
        if (now_speed < speed) {
            if (speed - now_speed < speed_delta) now_speed = speed;
            else now_speed += speed_delta;
        } else {
            if (now_speed - speed < speed_delta) now_speed = speed;
            else now_speed -= speed_delta;
        }
        if (now_speed > 0 && now_dir != 0) {
            digitalWrite(dir_pin1, LOW);
            digitalWrite(dir_pin2, HIGH);
            now_dir = 0;
        } else if (now_speed < 0 && now_dir != 1) {
            digitalWrite(dir_pin1, HIGH);
            digitalWrite(dir_pin2, LOW);
            now_dir = 1;
        }
        analogWrite(pwm_pin, min(abs(now_speed), 255));
    }
};

enum State { NONE = -1, STOP = 0, FORWARD = 1, TURN_RIGHT = 2 }; 
struct StateSequenceNode {
    State state;
    StateSequenceNode *next_state;
    StateSequenceNode() : state(NONE) {}
    virtual bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) { 
        return 0; 
    }
};
struct StopState : StateSequenceNode {
    StopState() : StateSequenceNode() {
        state = STOP;
    }
    virtual bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) { 
        return left_speed == 0 && right_speed == 0; 
    }
};
struct ForwardState : StateSequenceNode {
    int node_count, counter = 0;
    int now_on = 0; // 0 for line, 1 for node
    ForwardState(int node_count, int now_on) : node_count(node_count), counter(0), now_on(now_on) {
        state = FORWARD;
    }
    bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
        Serial.println(counter);
        int sum = 0;
        for (int i = 0; i < 5; ++i) sum += ir_result[i];
        if (sum >= 4) {
            if (now_on == 0) {
                now_on = 1, ++counter;
                if (counter == node_count) return true;    
            }
        } else now_on = 0; 
        return false;
    }
};
struct TurnRightState : StateSequenceNode {
    int line_count, counter = 0;
    int now_on = 0; // 0 for empty, 1 for line
    TurnRightState(int line_count, int now_on) : line_count(line_count), counter(0), now_on(now_on) {
        state = TURN_RIGHT;
    }
    bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
        if ((ir_result[1] || ir_result[2] || ir_result[3]) && !ir_result[4] && !ir_result[0]) {
            if (now_on == 0) {
                now_on = 1, ++counter;
                if (counter == line_count) return true;    
            }
        } else now_on = 0; 
        return false;
    }
};

class TreasureFindingCar {
private:
    DigitalIR digital_ir[5];
    double ir_weight[5] = {-7.0, -5.0, 0.0, 5.0, 7.0};
    Motor left_motor, right_motor; 
    double motor_speed_bias = 1 / 1.07;
    double proportion_const = 0.03;
    StateSequenceNode *now_state;
public:
    int ir_result[5];
    TreasureFindingCar() {
        // IR module setting up
        digital_ir[0] = DigitalIR(LEFT2);
        digital_ir[1] = DigitalIR(LEFT1);
        digital_ir[2] = DigitalIR(MIDDLE);
        digital_ir[3] = DigitalIR(RIGHT1);
        digital_ir[4] = DigitalIR(RIGHT2);
        // motor setting up
        right_motor = Motor(PWMA, AIN1, AIN2);
        left_motor = Motor(PWMB, BIN1, BIN2);
        // state
        now_state = new StopState();
        now_state->next_state = nullptr;
    }
    void setStateSequence(StateSequenceNode* state_sequence) {
        while (now_state != nullptr) {
            StateSequenceNode* now_node = now_state;
            now_state = now_state->next_state;
            delete now_node;
        }
        now_state = state_sequence;
    }
    void update() {
        detect();
        // state transitions
        if (now_state->checkStateEnd(ir_result, left_motor.now_speed, right_motor.now_speed)) {
            StateSequenceNode* now_node = now_state;
            now_state = now_state->next_state;
            delete now_node;
        }
        if (now_state->state == FORWARD) {
            forward(200);
        } else if (now_state->state == TURN_RIGHT) {
            turnRight(50);
        } else if (now_state->state == STOP) {
            stop();
        }
        lcd.setCursor(0, 0);
        lcd.print(line1);
    }
    void detect() {
        for (int i = 0; i < 5; ++i) ir_result[i] = digital_ir[i].read();
    }
    void forward(int speed) {
        int sum = 0;
        double weight_sum = 0;
        for (int i = 0; i < 5; ++i) sum += ir_result[i], weight_sum += ir_weight[i] * ir_result[i];
        double correction = sum ? proportion_const * speed * weight_sum / sum : 0;
        left_motor.setSpeed((speed + correction));
        right_motor.setSpeed((speed - correction) * motor_speed_bias);
    }
    void turnRight(int speed) {
        left_motor.setSpeed(speed);
        right_motor.setSpeed(-speed * motor_speed_bias);
    }
    void stop() {
        left_motor.setSpeed(0);
        right_motor.setSpeed(0);
    }
};

TreasureFindingCar car;

void setup() {
    Serial.begin(9600);
    StateSequenceNode* begin = new ForwardState(2, 0);
    begin->next_state = new StopState();
    begin->next_state->next_state = new TurnRightState(1, 0);
    begin->next_state->next_state->next_state = new StopState();
    begin->next_state->next_state->next_state->next_state = new ForwardState(1, 0);
    begin->next_state->next_state->next_state->next_state->next_state = new StopState();
    car.setStateSequence(begin);
    lcd.init();
    lcd.backlight();
}

void loop() {
    car.update();
    // if (state) return;
    // for (int speed = 0; speed <= 200; speed += 40) {
    //     car.forward(speed);
    //     delay(30);
    // }
    // delay(1000);
    // for (int speed = 200; speed >= 0; speed -= 40) {
    //     car.forward(speed);
    //     delay(30);
    // }
    // state++;
}