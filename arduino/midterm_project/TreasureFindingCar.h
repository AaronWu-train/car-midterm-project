#ifndef _TREASURE_FINDING_CAR_H_
#define _TREASURE_FINDING_CAR_H_

#include "MotorController.h"
#include "InfraredModule.h"
#include "MovementCommands.h"

class TreasureFindingCar {
private:
    // concrete hardware
    MotorController motor_controller;
    DigitalIR *digital_IR;

    // abstract
    MovementCommand *movement_commands;
public:
};

#include "TreasureFindingCar.cpp"

#endif // _TREASURE_FINDING_CAR_H_