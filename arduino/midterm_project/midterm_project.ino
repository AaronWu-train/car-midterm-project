#include <MFRC522.h>
#include <SPI.h>
#include "CarParameters.h"
#define ull unsigned long long

template <typename T>
class Queue
{
private:
    struct Node
    {
        T data;
        Node *next;
    } *m_front, *m_rear;
    size_t m_size;

public:
    Queue(void) : m_front(nullptr), m_rear(nullptr), m_size(0) {}
    ~Queue(void)
    {
        while (m_front != nullptr)
        {
            Node *temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
    }
    void push(const T &data)
    {
        Node *newNode = new Node{data, nullptr};
        if (m_rear != nullptr)
        {
            m_rear->next = newNode;
        }
        else
        {
            m_front = newNode;
        }
        m_rear = newNode;
        ++m_size;
    }
    void pop(void)
    {
        if (m_front != nullptr)
        {
            Node *temp = m_front;
            m_front = m_front->next;
            delete temp;
            --m_size;
            if (m_front == nullptr)
            {
                m_rear = nullptr;
            }
        }
    }
    T &front(void)
    {
        return m_front->data;
    }
    bool empty(void) const
    {
        return m_front == nullptr;
    }
    size_t size(void) const
    {
        return m_size;
    }
    void clear(void)
    {
        while (m_front != nullptr)
        {
            Node *temp = m_front;
            m_front = m_front->next;
            delete temp;
        }
        m_rear = nullptr;
        m_size = 0;
    }
};
struct State
{
    enum PossibleState
    {
        FORWARD = 0,
        TURN_LEFT = 1,
        TURN_RIGHT = 2,
        TURN_BACK = 3,
        STOP = 4
    } state;
    ull duration;
    State(int state, ull duration) : state(state), duration(duration) {}
};
class BluetoothTransmitter
{
private:
    const byte type_bitmask = 0b11100000, duration_bitmask = 0b00011111;
    const int type_bit_right_shift = 5, duration_magnifier = 20;

public:
    BluetoothTransmitter()
    {
        Serial1.begin(9600);
    }
    void sendRFIDCardUID(int uid_size, byte *uid)
    {
        Serial1.write('U');
        for (int i = 0; i < uid_size; ++i)
            Serial1.write((int)uid[i]);
    }
    void sendCarIsNowIdle()
    {
        Serial1.write('I');
    }
    bool checkRemoteCommandStreamInput(Queue<State> &state_queue)
    {
        if (!Serial1.available())
            return false;
        // we receive a command stream coming from remote Python program until we get 0b0 (end of command stream)
        byte command_byte, previous_command_byte = 0b000;
        do
        {
            command_byte = Serial1.read();
            if (command_byte == 0b11111111)
                continue; // we will get 255 if we try read the serial when it has no received bytes yet
            char command_type = (command_byte & type_bitmask) >> type_bit_right_shift;
            // int duration = duration_magnifier * (command_byte & duration_bitmask);
            if (command_type == 0b001)
            {
                if (previous_command_byte == 0b001 || previous_command_byte == 0b000)
                {
                    state_queue.push(State(State::PossibleState::FORWARD, forward_forward_duration));
                    Serial.println("forward");
                }
                else if (previous_command_byte == 0b010)
                {
                    state_queue.push(State(State::PossibleState::STOP, 0));
                    state_queue.push(State(State::PossibleState::FORWARD, turn_left_forward_duration));
                    Serial.println("forward(after left)");
                }
                else if (previous_command_byte == 0b011)
                {
                    state_queue.push(State(State::PossibleState::STOP, 0));
                    state_queue.push(State(State::PossibleState::FORWARD, turn_right_forward_duration));
                    Serial.println("forward(after right)");
                }
                else if (previous_command_byte == 0b100)
                {
                    state_queue.push(State(State::PossibleState::STOP, 0));
                    state_queue.push(State(State::PossibleState::FORWARD, turn_back_forward_duration));
                    Serial.println("forward(after back)");
                }
            }
            else if (command_type == 0b010)
            {
                state_queue.push(State(State::PossibleState::STOP, 0));
                state_queue.push(State(State::PossibleState::TURN_LEFT, turn_left_duration));
                Serial.println("turn left");
            }
            else if (command_type == 0b011)
            {
                state_queue.push(State(State::PossibleState::STOP, 0));
                state_queue.push(State(State::PossibleState::TURN_RIGHT, turn_right_duration));
                Serial.println("turn right");
            }
            else if (command_type == 0b100)
            {
                state_queue.push(State(State::PossibleState::STOP, 0));
                state_queue.push(State(State::PossibleState::TURN_BACK, turn_back_duration));
                Serial.println("turn back");
            }
            previous_command_byte = command_type;
        } while (command_byte);
        state_queue.push(State(State::PossibleState::STOP, 0));
        Serial.println("command stream ended, queue size: " + String(state_queue.size()));
        return true;
    }
};
struct DigitalIR
{
    int pin;
    DigitalIR() {}
    DigitalIR(int pin) : pin(pin) { pinMode(pin, INPUT); }
    int read() { return digitalRead(pin); }
};
struct Motor
{
    int pwm_pin, dir_pin1, dir_pin2; // motor controll pins
    int now_dir, now_speed;
    int speed_delta;
    Motor() {}
    Motor(int pwm_pin, int dir_pin1, int dir_pin2, int speed_delta) : pwm_pin(pwm_pin), dir_pin1(dir_pin1), dir_pin2(dir_pin2), speed_delta(speed_delta), now_dir(-1), now_speed(0)
    {
        pinMode(pwm_pin, OUTPUT);
        pinMode(dir_pin1, OUTPUT);
        pinMode(dir_pin2, OUTPUT);
    }
    void setSpeed(int speed)
    {
        if (now_speed < speed)
        {
            if (speed - now_speed < speed_delta)
                now_speed = speed;
            else
                now_speed += speed_delta;
        }
        else
        {
            if (now_speed - speed < speed_delta)
                now_speed = speed;
            else
                now_speed -= speed_delta;
        }
        if (now_speed > 0 && now_dir != 0)
        {
            digitalWrite(dir_pin1, LOW);
            digitalWrite(dir_pin2, HIGH);
            now_dir = 0;
        }
        else if (now_speed < 0 && now_dir != 1)
        {
            digitalWrite(dir_pin1, HIGH);
            digitalWrite(dir_pin2, LOW);
            now_dir = 1;
        }
        analogWrite(pwm_pin, min(abs(now_speed), 255));
    }
};
struct RFIDSensor
{
private:
    MFRC522 *mfrc522;

public:
    struct DetectionResult
    {
        bool detected;
        int uid_size;
        byte *uid;
    };
    RFIDSensor() {}
    void init(int ss_pin, int rst_pin)
    {
        SPI.begin();
        mfrc522 = new MFRC522(ss_pin, rst_pin);
        Serial.println("Read UID on a MIFARE PICC:");
    }
    RFIDSensor::DetectionResult detect()
    {
        // 靜態變數用來追蹤卡片是否仍在讀取區
        static bool cardProcessed = false;
        mfrc522->PCD_Init();

        // 檢查是否有新卡進入
        if (mfrc522->PICC_IsNewCardPresent())
        {
            // 如果新卡進入且尚未處理，則進行讀取
            if (!cardProcessed && mfrc522->PICC_ReadCardSerial())
            {
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
        }
        else
        {
            // 當感應區內無卡時，重置處理標記以便下次新卡進入時能再次讀取
            cardProcessed = false;
        }
        // 若未滿足讀取條件，回傳未偵測狀態
        return {false};
    }
};
class TreasureFindingCar
{
private:
    DigitalIR digital_ir[7];
    int ir_result[7];
    Motor left_motor, right_motor;
    RFIDSensor rfid_sensor;
    Queue<State> state_queue;
    ull current_state_start_time;
    BluetoothTransmitter bluetooth_transmitter;
    bool idle_signal_sent = false;

public:
    void init()
    {
        // IR module setting up
        digital_ir[0] = DigitalIR(LEFT3);
        digital_ir[1] = DigitalIR(LEFT2);
        digital_ir[2] = DigitalIR(LEFT1);
        digital_ir[3] = DigitalIR(MIDDLE);
        digital_ir[4] = DigitalIR(RIGHT1);
        digital_ir[5] = DigitalIR(RIGHT2);
        digital_ir[6] = DigitalIR(RIGHT3);
        // motor setting up
        left_motor = Motor(PWMB, BIN1, BIN2, motor_speed_maximum_difference);
        right_motor = Motor(PWMA, AIN1, AIN2, motor_speed_maximum_difference * motor_speed_bias);
        // RFID
        rfid_sensor.init(SS_PIN, RST_PIN);
        idle_signal_sent = false;
    }
    void update()
    {
        // RFID & bluetooth send
        RFIDSensor::DetectionResult rfid_res = rfid_sensor.detect();
        if (rfid_res.detected)
        {
            bluetooth_transmitter.sendRFIDCardUID(rfid_res.uid_size, rfid_res.uid);
        }
        // IR
        for (int i = 0; i < 7; ++i)
            ir_result[i] = digital_ir[i].read();
        for (int i = 0; i < 7; ++i) Serial.print(ir_result[i]);
        Serial.println("");
        // send idle if state queue is empty
        if (state_queue.empty())
        {
            if (!idle_signal_sent)
                bluetooth_transmitter.sendCarIsNowIdle(), idle_signal_sent = true;
            if (bluetooth_transmitter.checkRemoteCommandStreamInput(state_queue))
                idle_signal_sent = false, current_state_start_time = millis();
            return;
        }
        // state transistions
        State current_state = state_queue.front();
        bool timeUp = millis() - current_state_start_time >= current_state.duration;
        bool endMet = false;
        
        switch (current_state.state) {
            case State::PossibleState::FORWARD:
                endMet = forwardExtraEndCondition(ir_result, left_motor.now_speed, right_motor.now_speed);
                break;
            case State::PossibleState::TURN_LEFT:
                endMet = turnLeftExtraEndCondition(ir_result, left_motor.now_speed, right_motor.now_speed);
                break;
            case State::PossibleState::TURN_RIGHT:
                endMet = turnRightExtraEndCondition(ir_result, left_motor.now_speed, right_motor.now_speed);
                break;
            case State::PossibleState::TURN_BACK:
                endMet = turnBackExtraEndCondition(ir_result, left_motor.now_speed, right_motor.now_speed);
                break;
            case State::PossibleState::STOP:
                endMet = stopExtraEndCondition(ir_result, left_motor.now_speed, right_motor.now_speed);
                break;
        }
        if (timeUp && endMet) {
            state_queue.pop();
            current_state_start_time = millis();
        }
        if (current_state.state == State::PossibleState::FORWARD) {
            double baseSpeed = forward_speed;
            if (timeUp) baseSpeed *= 0.7;
            int sum = 0;
            double weight_sum = 0;
            for (int i = 0; i < 7; ++i) {
                sum += ir_result[i];
                weight_sum += ir_weight[i] * ir_result[i];
            }
            double correction = sum ? propotional_gain * baseSpeed * weight_sum / sum : 0;
            // correction = constrain(correction, -55.0, 55.0);

            left_motor.setSpeed(baseSpeed + correction);
            right_motor.setSpeed((baseSpeed - correction) * motor_speed_bias);
            return;
        } else if (current_state.state == State::PossibleState::TURN_LEFT) {
            double baseSpeed = turn_speed;
            if (timeUp) baseSpeed *= 0.7;
            int sum = 0;
            double weight_sum = 0;
            for (int i = 0; i < 7; ++i) {
                sum += ir_result[i];
                weight_sum += ir_weight[i] * ir_result[i];
            }
            if (timeUp && sum)
            {
                double corrected_speed = sum ? baseSpeed * weight_sum / sum / 10 : 0;
                if (0 <= corrected_speed < 55) corrected_speed = 55;
                if (-55 < corrected_speed < 0) corrected_speed = -55;
                left_motor.setSpeed(turn_speed_ratio * corrected_speed);
                right_motor.setSpeed(corrected_speed * motor_speed_bias);
            }
            else 
            {
                left_motor.setSpeed(turn_speed_ratio * baseSpeed);
                right_motor.setSpeed(baseSpeed * motor_speed_bias);
            }
        } else if (current_state.state == State::PossibleState::TURN_RIGHT) {
            double baseSpeed = turn_speed;
            if (timeUp) baseSpeed *= 0.7;
            int sum = 0;
            double weight_sum = 0;
            for (int i = 0; i < 7; ++i) {
                sum += ir_result[i];
                weight_sum += ir_weight[i] * ir_result[i];
            }
            if (timeUp && sum)
            {
                double corrected_speed = sum ? baseSpeed * weight_sum / sum / 10: 0;
                if (0 <= corrected_speed < 55) corrected_speed = 55;
                if (-55 < corrected_speed < 0) corrected_speed = -55;
                left_motor.setSpeed(corrected_speed);
                right_motor.setSpeed(corrected_speed * motor_speed_bias * turn_speed_ratio);
            }
            else 
            {
                left_motor.setSpeed(baseSpeed);
                right_motor.setSpeed(baseSpeed * motor_speed_bias * turn_speed_ratio);
            }
        } else if (current_state.state == State::PossibleState::TURN_BACK) {
            double baseSpeed = turn_speed;
            if (timeUp) baseSpeed *= 0.5;
            int sum = 0;
            double weight_sum = 0;
            for (int i = 0; i < 7; ++i) {
                sum += ir_result[i];
                weight_sum += ir_weight[i] * ir_result[i];
            }
            if (timeUp && sum)
            {
                double corrected_speed = sum ? baseSpeed * weight_sum / sum / 10: 0;
                if (0 <= corrected_speed < 55) corrected_speed = 55;
                if (-55 < corrected_speed < 0) corrected_speed = -55;
                left_motor.setSpeed(-corrected_speed);
                right_motor.setSpeed(corrected_speed * motor_speed_bias);
            }
            else 
            {
                left_motor.setSpeed(-baseSpeed);
                right_motor.setSpeed(baseSpeed * motor_speed_bias);
            }
        }
        else if (current_state.state == State::PossibleState::STOP)
        {
            left_motor.setSpeed(0);
            right_motor.setSpeed(0);
        }
    }
};

TreasureFindingCar treasure_finding_car;

void setup()
{
    Serial.begin(9600);
    Serial.println("---Treasure Finding Car---");
    treasure_finding_car.init();
}
void loop()
{
    treasure_finding_car.update();
}