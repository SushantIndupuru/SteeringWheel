#include <Arduino.h>
#include <SerialPacketFunctions.h>
#include <Structs.h>
#include <FixedPoint.h>
#include "ButtonEdge.h"
#include "Tm1637helper.h"
#include <Servo.h>

TM1637Helper display(2, 3);

constexpr unsigned long REVERSE_PACKET_INTERVAL = 50; // ~20Hz

Button rightIndicator(8);
Button leftIndicator(9);
Button headlights(10);
Button hazards(11);
Button wipers(12);
constexpr uint8_t BATTERY_LED = 13;

constexpr uint8_t BRAKE_SENSOR = PIN_A0;


constexpr uint8_t WIPER_SERVO_PIN = 5;
constexpr int WIPER_POS_MIN = 20;
constexpr int WIPER_POS_MAX = 160;
constexpr unsigned long WIPER_SWEEP_TIME_MS = 600;

Servo wiperServo;

void updateWiper(bool enabled) {
    static bool lastEnabled = false;
    static bool sweepingUp = true; // true is moving toward max
    static unsigned long sweepStart = 0;
    static int sweepFrom = WIPER_POS_MIN;
    static int sweepTo = WIPER_POS_MAX;

    if (!enabled) {
        if (lastEnabled) {
            wiperServo.write(WIPER_POS_MIN);
        }
        lastEnabled = false;
        return;
    }

    unsigned long now = millis();

    if (!lastEnabled) {
        sweepingUp = true;
        sweepFrom = WIPER_POS_MIN;
        sweepTo = WIPER_POS_MAX;
        sweepStart = now;
        wiperServo.write(WIPER_POS_MIN);
    }

    float t = (float) (now - sweepStart) / (float) WIPER_SWEEP_TIME_MS;
    if (t >= 1.0f) {
        sweepingUp = !sweepingUp;
        sweepFrom = sweepingUp ? WIPER_POS_MIN : WIPER_POS_MAX;
        sweepTo = sweepingUp ? WIPER_POS_MAX : WIPER_POS_MIN;
        sweepStart = now;
        t = 0.0f;
    }

    int pos = (int) (sweepFrom + t * (sweepTo - sweepFrom));
    wiperServo.write(pos);

    lastEnabled = true;
}

ForwardPacket latestForwardPacket{};
IndicatorState lastIndicatorState = INDICATOR_OFF;

void handlePacket(uint8_t type, uint8_t *data, uint8_t len) {
    if (type == 1 && len == sizeof(ForwardPacket)) {
        memcpy(&latestForwardPacket, data, sizeof(ForwardPacket));
    }
}

bool getBrakePedalState() {
    return analogRead(BRAKE_SENSOR) < 0.5; //TODO: get actual conversion formula and threshold
}

void setBatteryLed(float voltage) {
    digitalWrite(BATTERY_LED, voltage < 12.40); //TODO: get actual threshold
}

void setDisplaySpeed(uint8_t speed) {
    display.writeNumber(speed, false);
}

void setDisplayVoltage(float voltage) {
    display.writeFloat(12, 1);
}

void setup() {
    Serial.begin(115200);
    pinMode(BATTERY_LED, OUTPUT);
    digitalWrite(BATTERY_LED, LOW);
    rightIndicator.begin();
    leftIndicator.begin();
    headlights.begin();
    hazards.begin();
    wipers.begin();
    wiperServo.attach(WIPER_SERVO_PIN);
    wiperServo.write(WIPER_POS_MIN);
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

    float voltage = decodeFixedToNumber(latestForwardPacket.voltage);
    setDisplayVoltage(voltage);
    setBatteryLed(voltage);

    updateWiper(wipers.toggleState());

    switch (lastIndicatorState) {
        case RIGHT: {
            if (leftIndicator.toggleState() || hazards.toggleState()) {
                rightIndicator.resetToggle();
            }
            break;
        }
        case LEFT: {
            if (rightIndicator.toggleState() || hazards.toggleState()) {
                leftIndicator.resetToggle();
            }
            break;
        }
        case HAZARDS: {
            if (rightIndicator.toggleState() || leftIndicator.toggleState()) {
                hazards.resetToggle();
            }
            break;
        }
        default:
            break;
    }

    IndicatorState state = INDICATOR_OFF;
    if (hazards.toggleState()) {
        state = HAZARDS;
    } else if (leftIndicator.toggleState()) {
        state = LEFT;
    } else if (rightIndicator.toggleState()) {
        state = RIGHT;
    }
    lastIndicatorState = state;

    static unsigned long lastReverseSend = 0;
    unsigned long now = millis();
    if (now - lastReverseSend >= REVERSE_PACKET_INTERVAL) {
        lastReverseSend = now;
        ReversePacket packet = {state, headlights.toggleState(), getBrakePedalState(), true};
        sendPacket(Serial, 2, reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
    }
}
