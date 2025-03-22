#ifndef _INFRARED_MODULE_H_
#define _INFRARED_MODULE_H_

typedef class DigitalInfraredModule {
    public:
        int m_pin_number;
        DigitalInfraredModule(void);
        DigitalInfraredModule(int pin_number);
        bool value(void);
} DigitalIF;

typedef class AnalogInfraredModule {
    public:
        int m_pin_number;
        AnalogInfraredModule(void);
        AnalogInfraredModule(int pin_number);
        int value(void);
} AnalogIF;

class DigitalInfraredArray {
    public:
        DigitalInfraredArray(void);
        DigitalInfraredArray(int infrared_module_amount);
        DigitalInfraredArray(int infrared_module_amount, int *infrared_module_pins);
        int detect(void); // (infared_module_amount) bit integer, every bit represents the detection result of a IR module
    private:
        int m_infrared_module_amount;
        DigitalInfraredModule *m_infrared_modules;
};

#endif