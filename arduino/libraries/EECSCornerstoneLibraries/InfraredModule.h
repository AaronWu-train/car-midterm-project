#ifndef _INFRARED_MODULE_H_
#define _INFRARED_MODULE_H_

typedef class DigitalInfraredModule {
    public:
        int pin_number;
        DigitalInfraredModule(void);
        DigitalInfraredModule(int pin_number);
        bool value(void);
} DigitalIF;

typedef class AnalogInfraredModule {
    public:
        int pin_number;
        AnalogInfraredModule(void);
        AnalogInfraredModule(int pin_number);
        int value(void);
} AnalogIF;

class TrailTrackingFiveDigitalInfraredArray {
    public:
        TrailTrackingFiveDigitalInfraredArray(void);
        TrailTrackingFiveDigitalInfraredArray(int infrared_module_pins[5]);
        struct DetectionResult { int result_type, direction; };
        DetectionResult detect(void);
    private:
        DigitalInfraredModule infrared_modules[5];
        int history_infrared_values[20];
        
};

#endif