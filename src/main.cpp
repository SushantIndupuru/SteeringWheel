#include <Arduino.h>
#include <SerialPacketFunctions.h>
#include <Structs.h>
#include <FixedPoint.h>
#include "ButtonEdge.h"
#include "Tm1637helper.h"

TM1637Helper display(2, 3);

constexpr unsigned long REVERSE_PACKET_INTERVAL = 50; // ~20Hz

Button rightIndicator(8);
Button leftIndicator(9);
Button headlights(10);
Button hazards(11);
Button wipers(12);
constexpr uint8_t LEFT_INDICATOR = 13;
ForwardPacket latestForwardPacket{};

void handlePacket(uint8_t type, uint8_t *data, uint8_t len) {
    if (type == 1 && len == sizeof(ForwardPacket)) {
        memcpy(&latestForwardPacket, data, sizeof(ForwardPacket));
    }
}

void setBatterLed(bool state) {
    digitalWrite(LEFT_INDICATOR, state);
}

void setDisplaySpeed(uint8_t speed) {
    display.writeNumber(speed,false);
}

void setDisplayVoltage(float voltage) {
    display.writeFloat(voltage, 2);
}

void setup() {
    Serial.begin(115200);
    rightIndicator.begin();
    leftIndicator.begin();
    headlights.begin();
    hazards.begin();
    wipers.begin();
    display.begin();
}

void loop() {
    rightIndicator.update();
    rightIndicator.toggle();
    leftIndicator.update();
    leftIndicator.toggle();
    headlights.update();
    headlights.toggle();
    hazards.update();
    hazards.toggle();
    wipers.update();
    wipers.toggle();

    updatePacket(Serial, handlePacket);

    setDisplayVoltage(decodeFixedToNumber(latestForwardPacket.voltage));

    static unsigned long lastReverseSend = 0;
    unsigned long now = millis();
    IndicatorState state = INDICATOR_OFF;
    if (hazards.toggleState()) {
        state = HAZARDS;
    } else if (leftIndicator.toggleState()) {
        state = LEFT;
    } else if (rightIndicator.toggleState()) {
        state = RIGHT;
    }
    if (now - lastReverseSend >= REVERSE_PACKET_INTERVAL) {
        lastReverseSend = now;
        ReversePacket packet = {state, headlights.toggleState(), 0, 1};
        sendPacket(Serial, 1, reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
    }
}
