#include <MFRC522.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  //SCL->A5, SDA->A4, VCC->5V
char line1[16] = "counter:        ";
char line2[16] = "now on:         ";

const int PWMA =7, AIN1 = 11, AIN2 = 8; // Right motor
const int PWMB = 10, BIN1 = 12, BIN2 = 13; // Left motor
const int LEFT2 = A8, LEFT1 = A9, MIDDLE = A10, RIGHT1 = A11, RIGHT2 = A12; // IR modules
const int RST_PIN = 6, SS_PIN = 53; // RFID

struct DigitalIR {
    int pin;
    DigitalIR() {}
    DigitalIR(int pin) : pin(pin) { pinMode(pin, INPUT); }
    int read() { return digitalRead(pin); }
};
struct Motor {
    int pwm_pin, dir_pin1, dir_pin2; // motor controll pins
    int now_dir, now_speed;
    int speed_delta;
    Motor(){}
    Motor(int pwm_pin, int dir_pin1, int dir_pin2, int speed_delta = 20) : pwm_pin(pwm_pin), dir_pin1(dir_pin1), dir_pin2(dir_pin2), speed_delta(speed_delta), now_dir(-1), now_speed(0) {
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

enum State { NONE = -1, STOP = 0, FORWARD = 1, TURN_RIGHT = 2, TURN_LEFT = 3, TURN_BACK = 4 }; 
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
  const unsigned long duration;
  unsigned long startTime;  // 記錄開始時間
  bool timingStarted;       // 判斷是否已啟動計時

  
  ForwardState(bool isPostTurn)
    : duration(isPostTurn ? 400UL : 580UL), startTime(0), timingStarted(false) {
    state = FORWARD;
  }

  bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
    if (!timingStarted) {
      startTime = millis();
      timingStarted = true;
    }
    return (millis() - startTime >= duration);
  }
};
struct TurnRightState : StateSequenceNode {
    
  const unsigned long duration = 400;
  unsigned long startTime;  // 記錄計時開始的時間
  bool timingStarted;       // 記錄是否已開始計時

  // 預設建構式，不需要傳入參數
  TurnRightState()
    : startTime(0), timingStarted(false) {
    state = TURN_RIGHT;
  }
  TurnRightState(int line_count, int now_on)
    : startTime(0), timingStarted(false) {
    state = TURN_RIGHT;
  }

  // 每次呼叫此函數時，檢查是否已經達到 1 秒
  bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
    if (!timingStarted) {
      startTime = millis();
      timingStarted = true;
    }
    if (millis() - startTime >= duration ) {
        if (ir_result[2] || ir_result[3] || ir_result[4]) return true;
        return false;
    }
    return false;
    // 當經過的毫秒數大於等於 duration (1000 毫秒) ，回傳 true 表示狀態結束
    return (millis() - startTime >= duration);
  }
};
struct TurnLeftState : StateSequenceNode {
    // 固定右轉持續時間：1 秒 = 1000 毫秒
  const unsigned long duration = 400;
  unsigned long startTime;  // 記錄計時開始的時間
  bool timingStarted;       // 記錄是否已開始計時

  // 預設建構式，不需要傳入參數
  TurnLeftState()
    : startTime(0), timingStarted(false) {
    state = TURN_LEFT;
  }
  TurnLeftState(int line_count, int now_on)
    : startTime(0), timingStarted(false) {
    state = TURN_LEFT;
  }

  // 每次呼叫此函數時，檢查是否已經達到 1 秒
  bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
    if (!timingStarted) {
      startTime = millis();
      timingStarted = true;
    }
    if (millis() - startTime >= duration ) {
        if (ir_result[2] || ir_result[3] || ir_result[4]) return true;
        return false;
    }
    // 當經過的毫秒數大於等於 duration (1000 毫秒) ，回傳 true 表示狀態結束
    return false;
  }
};

struct TurnBackState : StateSequenceNode {
  const unsigned long duration = 650;  // U-turn 持續 2000 毫秒 (2秒)
  unsigned long startTime;              // 記錄開始時間
  bool timingStarted;                   // 判斷是否已啟動計時

  // 預設建構式
  TurnBackState() : startTime(0), timingStarted(false) {
      state = TURN_BACK;
  }
  TurnBackState(int line_count, int now_on) : startTime(0), timingStarted(false) {
      state = TURN_BACK;
  }
  
  // 檢查是否達到迴轉持續時間
  bool checkStateEnd(int ir_result[5], int left_speed, int right_speed) {
      if (!timingStarted) {
          startTime = millis();
          timingStarted = true;
      }
      return (millis() - startTime >= duration);
  }
};


struct RFIDSensor {
private:
    MFRC522* mfrc522;
public:
    struct DetectionResult {
        bool detected;
        int uid_size;
        byte* uid;
    };
    RFIDSensor() {}
    void init(int ss_pin, int rst_pin) {
        SPI.begin();
        mfrc522 = new MFRC522(ss_pin, rst_pin);
        Serial.println("Read UID on a MIFARE PICC:");
    }
    RFIDSensor::DetectionResult detect() {
        // 靜態變數用來追蹤卡片是否仍在讀取區
        static bool cardProcessed = false;
        mfrc522->PCD_Init();

        // 檢查是否有新卡進入
        if(mfrc522->PICC_IsNewCardPresent()) {
            // 如果新卡進入且尚未處理，則進行讀取
            if(!cardProcessed && mfrc522->PICC_ReadCardSerial()) {
                cardProcessed = true;
                Serial.println(F("Card Detected:"));
                RFIDSensor::DetectionResult result;
                result.detected = true;
                result.uid = mfrc522->uid.uidByte;
                result.uid_size = mfrc522->uid.size;

                // 呼叫 Halt 與 StopCrypto1 結束本次讀取狀態
                mfrc522->PICC_HaltA();
                mfrc522->PCD_StopCrypto1();

                return result;
            }
        } else {
            // 當感應區內無卡時，重置處理標記以便下次新卡進入時能再次讀取
            cardProcessed = false;
        }
        // 若未滿足讀取條件，回傳未偵測狀態
        return {false};
    }
    // DetectionResult detect() {
    //     if(!mfrc522->PICC_IsNewCardPresent()) return {false};
    //     if(!mfrc522->PICC_ReadCardSerial()) return {false};
    //     Serial.println(F("**Card Detected:**"));
    //     DetectionResult result = {true};
    //     result.uid = mfrc522->uid.uidByte;
    //     result.uid_size = mfrc522->uid.size;
    //     mfrc522->PICC_HaltA();
    //     mfrc522->PCD_StopCrypto1();
    //     return result;
    // }
};

// current hardware is using Serial1 as its communication port
class BluetoothTransmitter {
private:
    char command_str[100];
    int command_len;
public:
    struct ReceivedCommand {
        bool received;
        StateSequenceNode* command;
    };
    BluetoothTransmitter() {
        Serial1.begin(9600);
    }
    void sendCardUID(int uid_size, byte* uid) {
        Serial1.write('U');
        for (int i = 0; i < uid_size; ++i) {
            Serial.println((int)uid[i]);
            Serial1.write((int)uid[i]);
        }
    }
    void sendIdle() {
        Serial1.write('I');
    }
    ReceivedCommand receiveCommand() {
        if (!Serial1.available()) return {false, nullptr};
        // command format: one byte as one command
        // first four bit: FRLS, mutual exclusive, 1111 represent command stream ended
        // e.g. 1000 -> forward, 0010 turn right
        // last four bit: BCD count
        // e.g. 1000 0110 -> forward for 6 node
        // e.g. 0100 0010 -> turn right for two lines
        // every command stream should be ended with command stream ended
        byte cmd_byte;
        StateSequenceNode *first = nullptr, *last = nullptr, *temp;
        while (true) {
            cmd_byte = Serial1.read();
            Serial.println(cmd_byte);
            if (cmd_byte == 0b00000000) {
                Serial.println("command stream terminate.");
                temp = new StateSequenceNode();
                last->next_state = temp;
                last = temp;
                break;
            }
            else if (cmd_byte & 0b10000000 && !(cmd_byte & 0b01111000)) {
                Serial.print("forward for ");
                Serial.print(cmd_byte & 0b00001111);
                Serial.println(" steps.");
                bool isPostTurn = false;
                if (last != nullptr && (last->state == TURN_RIGHT || last->state == TURN_LEFT))
                  isPostTurn = true;
                if (first == nullptr) first = last = new ForwardState(isPostTurn);
                else {
                  temp = new ForwardState(isPostTurn);
                  last->next_state = temp;
                  last = temp;
                }
            } else if (cmd_byte & 0b01000000 && !(cmd_byte & 0b10111000)) {
                Serial.print("right for ");
                Serial.print(cmd_byte & 0b00000111);
                Serial.println(" 90 degrees.");
                if (first == nullptr) first = last = new TurnRightState(cmd_byte & 0b00000111, 0);
                else {
                    temp = new TurnRightState(cmd_byte & 0b00000111, 0);
                    last->next_state = temp;
                    last = temp;
                }
            } else if (cmd_byte & 0b00100000 && !(cmd_byte & 0b11011000)) {
                Serial.print("left for ");
                Serial.print(cmd_byte & 0b00000111);
                Serial.println(" 90 degrees.");
                if (first == nullptr) first = last = new TurnLeftState(cmd_byte & 0b00000111, 0);
                else {
                    temp = new TurnLeftState(cmd_byte & 0b00000111, 0);
                    last->next_state = temp;
                    last = temp;
                }
            } else if (cmd_byte & 0b00010000 && !(cmd_byte & 0b11101000)) {
                Serial.print("wait until two wheels are stopped");
                if (first == nullptr) first = last = new StopState();
                else {
                    temp = new StopState();
                    last->next_state = temp;
                    last = temp;
                }
            } else if (cmd_byte & 0b00001000 && !(cmd_byte & 0b11110000)) {
                Serial.print("turn back");
                if (first == nullptr) first = last = new TurnBackState(1, 0);
                else {
                    temp = new TurnBackState(1, 0);
                    last->next_state = temp;
                    last = temp;
                }
            }
        }
        return {true, first};
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
    RFIDSensor rfid_sensor;
    BluetoothTransmitter bluetooth_transmitter;
    bool idle_signal_sent;
public:
    int ir_result[5];
    void init() {
        // IR module setting up
        digital_ir[0] = DigitalIR(LEFT2);
        digital_ir[1] = DigitalIR(LEFT1);
        digital_ir[2] = DigitalIR(MIDDLE);
        digital_ir[3] = DigitalIR(RIGHT1);
        digital_ir[4] = DigitalIR(RIGHT2);
        // motor setting up
        left_motor = Motor(PWMB, BIN1, BIN2, 40);
        right_motor = Motor(PWMA, AIN1, AIN2, 40 * motor_speed_bias);
        // state
        now_state = new StateSequenceNode();
        now_state->next_state = nullptr;
        // RFID
        rfid_sensor.init(SS_PIN, RST_PIN);
        idle_signal_sent = false;
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
        // bluetooth
        if (now_state->state == NONE) {
            if (!idle_signal_sent) {
                bluetooth_transmitter.sendIdle();
                idle_signal_sent = true;
            }
            BluetoothTransmitter::ReceivedCommand cmd = bluetooth_transmitter.receiveCommand();
            if (cmd.received) {
                now_state = cmd.command;
                idle_signal_sent = false;
            }
        }
        // rfid
        RFIDSensor::DetectionResult rfid_res = rfid_sensor.detect();
        if (rfid_res.detected) {
            bluetooth_transmitter.sendCardUID(rfid_res.uid_size, rfid_res.uid);
        }
        // ir
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
            turnRight(100);
        } else if (now_state->state == TURN_LEFT) {
            turnLeft(100);
        } else if (now_state->state == STOP) {
            stop();
        } else if (now_state->state == TURN_BACK) {
            turnBack(100);
        }
    }
    void detect() {
        for (int i = 0; i < 5; ++i) ir_result[i] = digital_ir[i].read();
    }
    void forward(int speed) {
        int sum = 0;
        double weight_sum = 0;
        for (int i = 0; i < 5; ++i) sum += ir_result[i], weight_sum += ir_weight[i] * ir_result[i];
        double correction = sum ? proportion_const * speed * weight_sum / sum : 0;

        double maxCorrection = 30.0;
        if (correction > maxCorrection)
            correction = maxCorrection;
        else if (correction < -maxCorrection)
            correction = -maxCorrection;

        left_motor.setSpeed((speed + correction));
        right_motor.setSpeed((speed - correction) * motor_speed_bias);
    }
    void turnLeft(int speed) {
        left_motor.setSpeed(0.3 * speed);
        right_motor.setSpeed(speed * motor_speed_bias);
    }
    void turnRight(int speed) {
        left_motor.setSpeed(speed);
        right_motor.setSpeed(0.3 * speed * motor_speed_bias);
    }
    void turnBack(int speed) {
        left_motor.setSpeed(speed);
        right_motor.setSpeed(-speed * motor_speed_bias);
    }
    void stop() {   
        left_motor.setSpeed(0);
        right_motor.setSpeed(0);
    }
};

TreasureFindingCar car;

RFIDSensor rfid;
void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    car.init();
}

void loop() {
    car.update();
    // delay(5);
}