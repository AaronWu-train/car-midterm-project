#include <MFRC522.h>
#include <SPI.h>
#define ull unsigned long long


template<typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
    } *m_front, m_rear
    size_t m_size;
public:
    Queue(void) : m_front(nullptr), m_rear(nullptr), m_size(0) {}
    ~Queue(void) {
        while (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
    }
    void push(const T& data) {
        Node* newNode = new Node{data, nullptr};
        if (m_rear != nullptr) {
            m_rear->next = newNode;
        } else {
            m_front = newNode;
        }
        m_rear = newNode;
        ++m_size;
    }
    void pop(void) {
        if (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
            --m_size;
            if (m_front == nullptr) {
                m_rear = nullptr;
            }
        }
    }
    T& front(void) {
        if (m_front != nullptr) {
            return m_front->data;
        } else {
            throw std::out_of_range("Queue is empty");
        }
    }
    bool empty(void) const {
        return m_front == nullptr;
    }
    size_t size(void) const {
        return m_size;
    }
    void clear(void) {
        while (m_front != nullptr) {
            Node* temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
        m_rear = nullptr;
        m_size = 0;
    }
};
struct State {
    enum {
        FORWARD, 
        TURN_LEFT, 
        TURN_RIGHT,
        TURN_BACK, 
        STOP
    } state;
    ull duration;
}
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
};
class BluetoothTransmitter {
private:
    char command_str[100];
    int command_len;
public:
      
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
                if (first == nullptr) first = last = new ForwardState(cmd_byte & 0b00000111, 0);
                else {
                    temp = new ForwardState(cmd_byte & 0b00000111, 0);
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
            forward(100);
        } else if (now_state->state == TURN_RIGHT) {
            turnRight(50);
        } else if (now_state->state == TURN_LEFT) {
            turnLeft(50);
        } else if (now_state->state == STOP) {
            stop();
        } else if (now_state->state == TURN_BACK) {
            turnBack(50);
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


void setup() {

}
void loop() {

}