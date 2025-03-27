#ifndef _INFRARED_MODULE_H_
#define _INFRARED_MODULE_H_

class DigitalIR {
    private:
        int pin_number;
    public:
        DigitalIR(int pin_number_) : pin_number(pin_number_) {}
        bool detect() {
            return digitalRead(pin_number);
        }
};

class AnalogIR {
    private:
        int pin_number;
    public:
        AnalogIR(int pin_number_) : pin_number(pin_number_) {}
        int detect() {
            return analogRead(pin_number);
        }
};
#endif