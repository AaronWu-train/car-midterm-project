#ifndef _TRACKING_CONTRALLER_H_
#define _TRACKING_CONTRALLER_H_

#include "InfraredModule.h"
#include "MotorController.h"

class DigitalTrackingController {
    private:
        DigitalInfraredArray* m_infrared_array;
        MotorController* m_motor_controller;
    public:
        DigitalTrackingController(DigitalInfraredArray* m_infrared_array, MotorController* m_motor_controller);
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class ForwardNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class ForwardProportionDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class ForwardProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact, m_differential_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnLeftNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnLeftProportionDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnLeftProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact, m_differential_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnRightNoDigitalTrackingController : public DigitalTrackingController {
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnRightProportionDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};

class TurnRightProportionDifferentialDigitalTrackingController : public DigitalTrackingController {
    private:
        double m_proportion_impact, m_differential_impact;
    public:
        void instantUpdate(int speed); // use the value of infrared array to control motor
        void smoothUpdate(int speed); // use the value of infrared array and current motor speed to control motor
};


#endif