#ifndef SERIAL_STRUCTS_H
#define SERIAL_STRUCTS_H

#include <Arduino.h>

// packet type 1 – data from bulkhead to steeringwheel
struct Data {
    int8_t speed;
} __attribute__((packed));

// packet type 2 – debug mode
struct MotorCommand {
    uint8_t motorId;
    int16_t speed;
} __attribute__((packed));


#endif