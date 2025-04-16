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
    enum PossibleState{
        FORWARD = 0, 
        TURN_LEFT = 1, 
        TURN_RIGHT = 2,
        TURN_BACK = 3, 
        STOP = 4
    } state;
    ull duration;
    State(int state, ull duration) : state(state), duration(duration) {}
};
class CarParameterHolder {
public:
    // pins
    const int PWMA = 7, AIN1 = 11, AIN2 = 8; // Right motor
    const int PWMB = 10, BIN1 = 12, BIN2 = 13; // Left motor
    const int LEFT2 = A8, LEFT1 = A9, MIDDLE = A10, RIGHT1 = A11, RIGHT2 = A12; // IR modules
    const int RST_PIN = 6, SS_PIN = 53; // RFID
    // timing of movements
    ull forward_forward_duration = 580;
    ull turn_left_forward_duration = 200;
    ull turn_right_forward_duration = 200;
    ull turn_back_forward_duration = 200;
    ull turn_left_duration = 100;
    ull turn_right_duration = 100;
    ull turn_back_duration = 650;
    // motor
    double motor_speed_bias = 1 / 1.07; // right motor speed divided by left motor speed
    int motor_speed_maximum_difference = 100;
    // tracking
    
};
class BluetoothTransmitter {
private:
    const byte type_bitmask = 0b11000000, duration_bitmask = 0b00111111;
    const int type_bit_right_shift = 6, duration_magnifier = 20;
public:
    BluetoothTransmitter() { 
        Serial1.begin(9600); 
    }
    void sendRFIDCardUID(int uid_size, byte* uid) {
        Serial1.write('U'); 
        for (int i = 0; i < uid_size; ++i) Serial1.write((int)uid[i]); 
    }
    void sendCarIsNowIdle() { 
        Serial1.write('I'); 
    }
    bool checkRemoteCommandStreamInput(Queue<State> &state_queue, CarParameterHolder &hardware_parameter_holder) {
        if (!Serial1.available()) return false;
        // we receive a command stream coming from remote Python program until we get 0b0 (end of command stream)
        byte command_byte, previous_command_byte = 0b0;
        do {
            command_byte = Serial1.read();
            if (command_byte == 0b11111111) continue; // we will get 255 if we try read the serial when it has no received bytes yet
            char command_type = (command_byte & type_bitmask) >> type_bit_right_shift;
            // int duration = duration_magnifier * (command_byte & duration_bitmask);
            if (command_type == 0b00) {
                if (previous_command_byte == 0b00) {
                    state_queue.push(State(State::PossibleState::FORWARD, hardware_parameter_holder.forward_forward_duration));
                } else if (previous_command_byte == 0b01) {
                    state_queue.push(State(State::PossibleState::FORWARD, hardware_parameter_holder.turn_left_forward_duration));
                } else if (previous_command_byte == 0b10) {
                    state_queue.push(State(State::PossibleState::FORWARD, hardware_parameter_holder.turn_right_forward_duration));
                } else if (previous_command_byte == 0b11) {
                    state_queue.push(State(State::PossibleState::FORWARD, hardware_parameter_holder.turn_back_forward_duration));
                }
            } else if (command_type == 0b01) {
                state_queue.push(State(State::PossibleState::TURN_LEFT, hardware_parameter_holder.turn_left_duration))
            } else if (command_type == 0b10) {
                state_queue.push(State(State::PossibleState::TURN_RIGHT, hardware_parameter_holder.turn_right_duration));
            } else if (command_type == 0b11) {
                state_queue.push(State(State::PossibleState::TURN_BACK, hardware_parameter_holder.turn_back_duration));
            }
            previous_command_byte = command_byte;
        } while (command_byte);
        return true;
    }
};

class TreasureFindingCar {
private:
    Queue<State> state_queue;
public:
    

};

void setup() {

}
void loop() {

}