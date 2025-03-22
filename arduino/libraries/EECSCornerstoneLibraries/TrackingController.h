#ifndef _TRACKING_CONTRALLER_H_
#define _TRACKING_CONTRALLER_H_

#include "InfraredModule.h"
#include "MotorController.h"

class DigitalTrackingController {
    private:
        DigitalInfraredArray infrared_array;
        MotorController motor_controller;
    public:
        void update(int speed); // use the value of infrared array to control motor
};

class ForwardNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class ForwardProportionDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class ForwardProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnLeftNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnLeftProportionDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnLeftProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnRightNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnRightProportionDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};

class TurnRightProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    public:
        void update(int speed);
};


#endif