    #include <SerialPacketFunctions.h>
    #include "Tm1637helper.h"
    #include "ButtonEdge.h"
    #include <FixedPoint.h>
    #include <Arduino.h>
    #include <Structs.h>
    #include <Servo.h>
    #include <TM1637Display.h>
    #include "OledDisplay.h"

    //constants
    constexpr int WIPER_POS_MIN = 21;
    constexpr int WIPER_POS_MAX = 50;
    constexpr unsigned long WIPER_SWEEP_TIME_MS = 500;
    constexpr unsigned long BATTERY_FLASH_INTERVAL_MS = 350;
    constexpr unsigned long REVERSE_PACKET_INTERVAL = 50; // ~20Hz
    constexpr unsigned long DEBUG_COMBO_WINDOW_MS = 500;

    //hardware
    TM1637Helper display2(2, 3);
    TM1637Helper display(A2, A3);
    Button rightIndicator(8);
    Button leftIndicator(9);
    Button headlights(10);
    Button hazards(11);
    Button wipers(12);
    Button start(6);
    Button stop(7);
    Servo wiperServo;
    constexpr uint8_t BATTERY_LED = 13;
    constexpr uint8_t BRAKE_SENSOR = A0;
    constexpr uint8_t WIPER_SERVO_PIN = 5;

    //states
    enum DebugStates : uint8_t {
        VOLTAGE,
        BRAKE_SENSOR_READ,
        DISPLAY_TEST
    };

    enum EngineState : uint8_t {
        OFF,
        PRIMING,
        STARTING,
        ON
    };

    bool debugMode = false;
    bool starterState = false;
    bool ignitionState = false;
    unsigned long hazardsPressedAt = 0;
    unsigned long headlightsPressedAt = 0;
    unsigned long wipersPressedAt = 0;
    unsigned long engineStartTime = 0;
    EngineState engineState = OFF;

    //packet
    ForwardPacket latestForwardPacket{};

    bool checkDebugCombo() {
        const unsigned long now = millis();

        if (hazards.wasPressed()) hazardsPressedAt = now;
        if (headlights.wasPressed()) headlightsPressedAt = now;
        if (wipers.wasPressed()) wipersPressedAt = now;

        if (hazardsPressedAt > 0 && headlightsPressedAt > 0 && wipersPressedAt > 0) {
            const unsigned long earliest = min(hazardsPressedAt, min(headlightsPressedAt, wipersPressedAt));
            const unsigned long latest = max(hazardsPressedAt, max(headlightsPressedAt, wipersPressedAt));

            if (latest - earliest <= DEBUG_COMBO_WINDOW_MS && now - latest < DEBUG_COMBO_WINDOW_MS) {
                debugMode = !debugMode;
                hazards.resetToggle();
                headlights.resetToggle();
                wipers.resetToggle();
                rightIndicator.resetToggle();
                leftIndicator.resetToggle();
                hazardsPressedAt = headlightsPressedAt = wipersPressedAt = 0;
                return true;
            }
            if (now - earliest > DEBUG_COMBO_WINDOW_MS) {
                hazardsPressedAt = headlightsPressedAt = wipersPressedAt = 0;
            }
        }
        return false;
    }


    void handleButtons() {
        rightIndicator.toggle();
        leftIndicator.toggle();
        headlights.toggle();
        hazards.toggle();
        wipers.toggle();
        start.toggle();
        stop.toggle();
    }


    void updateWiper(const bool enabled) {
        static bool lastEnabled = false;
        static bool sweepingUp = true; //true is moving toward max
        static unsigned long sweepStart = 0;
        static int sweepFrom = WIPER_POS_MIN;
        static int sweepTo = WIPER_POS_MAX;

        if (!enabled) {
            if (lastEnabled) wiperServo.write(WIPER_POS_MIN);
            lastEnabled = false;
            return;
        }

        const unsigned long now = millis();

        if (!lastEnabled) {
            sweepingUp = true;
            sweepFrom = WIPER_POS_MIN;
            sweepTo = WIPER_POS_MAX;
            sweepStart = now;
            wiperServo.write(WIPER_POS_MIN);
        }

        float t = static_cast<float>(now - sweepStart) / static_cast<float>(WIPER_SWEEP_TIME_MS);
        if (t >= 1.0f) {
            sweepingUp = !sweepingUp;
            sweepFrom = sweepingUp ? WIPER_POS_MIN : WIPER_POS_MAX;
            sweepTo = sweepingUp ? WIPER_POS_MAX : WIPER_POS_MIN;
            sweepStart = now;
            t = 0.0f;
        }

        wiperServo.write(static_cast<int>(sweepFrom + t * (sweepTo - sweepFrom)));
        lastEnabled = true;
    }


    void handlePacket(const uint8_t type, const uint8_t *data, const uint8_t len) {
        if (type == 1 && len == sizeof(ForwardPacket)) {
            memcpy(&latestForwardPacket, data, sizeof(ForwardPacket));
        }
    }

    bool getBrakePedalState() {
        return analogRead(BRAKE_SENSOR) < 630; //TODO: get actual threshold
    }

    void setBatteryLed(const float voltage) {
        if (voltage < 12.30f) {
            digitalWrite(BATTERY_LED, millis() / BATTERY_FLASH_INTERVAL_MS % 2 == 0);
        } else if (voltage < 12.40f) {
            digitalWrite(BATTERY_LED, HIGH);
        } else {
            digitalWrite(BATTERY_LED, LOW);
        }
    }

    void setup() {
        Serial.begin(9600);
        Wire.begin();
        display.begin();
        display.setBrightness(7);
        display2.begin();
        display2.setBrightness(7);
        pinMode(BATTERY_LED, OUTPUT);
        digitalWrite(BATTERY_LED, HIGH);
        rightIndicator.begin();
        leftIndicator.begin();
        headlights.begin();
        hazards.begin();
        wipers.begin();
        start.begin();
        stop.begin();
        wiperServo.attach(WIPER_SERVO_PIN);
        wiperServo.write(WIPER_POS_MIN);

        static constexpr uint8_t startupSegs[4] = {
            0x00,
            SEG_D | SEG_E | SEG_F | SEG_G, // t
            SEG_A | SEG_D | SEG_E | SEG_F | SEG_G, // E
            SEG_A | SEG_D | SEG_E | SEG_F, // C
        };
        display.writeRaw(startupSegs);
        display2.writeRaw(startupSegs);
        delay(3000);
        digitalWrite(BATTERY_LED, LOW);
    }

    void loop() {
        static DebugStates lastDebugState = VOLTAGE;
        static IndicatorState lastIndicatorState = INDICATOR_OFF;

        rightIndicator.update();
        leftIndicator.update();
        headlights.update();
        hazards.update();
        wipers.update();
        stop.update();
        start.update();

        const bool comboFired = checkDebugCombo();

        updatePacket(Serial, handlePacket);

        const float voltage = decodeFixedToNumber(latestForwardPacket.voltage);
        setBatteryLed(voltage);
        display2.writeFloat(decodeFixedToNumber(latestForwardPacket.temperature));

        if (stop.wasPressed()) {
            engineState = OFF;
            ignitionState = false;
            starterState = false;
            start.resetToggle(); // prevent auto-restart since toggleState() stays true
        }

        switch (engineState) {
            case OFF:
                if (start.toggleState()) {
                    ignitionState = true;
                    starterState = false;
                    engineStartTime = millis();
                    engineState = PRIMING;
                }

                break;
            case PRIMING: {
                ignitionState = true;
                starterState = false;
                const unsigned long elapsed = millis() - engineStartTime;
                if (elapsed >= 1500) {
                    engineState = STARTING;
                }
                break;
            }
            case STARTING: {
                ignitionState = true;
                starterState = true;
                const unsigned long elapsed = millis() - engineStartTime;
                if (elapsed >= 3500) {
                    engineState = ON;
                }
                break;
            }
            case ON:
                ignitionState = true;
                starterState = false;
                break;
        }



        if (debugMode) {
            if (!comboFired) {
                handleButtons();
            }

            updateWiper(false);

            if (headlights.wasPressed()) {
                lastDebugState = VOLTAGE;
            }
            if (wipers.wasPressed()) {
                lastDebugState = BRAKE_SENSOR_READ;
            }
            if (hazards.wasPressed()) {
                lastDebugState = DISPLAY_TEST;
            }
            if (leftIndicator.held() && rightIndicator.held()) {
                //starterState = true;
            }

            switch (lastDebugState) {
                case VOLTAGE:
                    display.writeFloat(voltage, 2);
                    break;
                case BRAKE_SENSOR_READ:
                    display.writeNumber(analogRead(BRAKE_SENSOR), true);
                    break;
                case DISPLAY_TEST:
                    //display.writeFloat(88.88, 2);
                    display.writeFloat(decodeFixedToNumber(latestForwardPacket.temperature));
                    break;
            }
        } else {
            if (!comboFired) {
                handleButtons();
            }

            switch (lastIndicatorState) {
                case RIGHT:
                    if (leftIndicator.toggleState() || hazards.toggleState()) rightIndicator.resetToggle();
                    break;
                case LEFT:
                    if (rightIndicator.toggleState() || hazards.toggleState()) leftIndicator.resetToggle();
                    break;
                case HAZARDS:
                    if (rightIndicator.toggleState() || leftIndicator.toggleState()) hazards.resetToggle();
                    break;
                default:
                    break;
            }

            IndicatorState state = INDICATOR_OFF;
            if (hazards.toggleState()) state = HAZARDS;
            else if (leftIndicator.toggleState()) state = LEFT;
            else if (rightIndicator.toggleState()) state = RIGHT;
            lastIndicatorState = state;

            updateWiper(wipers.toggleState());

            display.writeNumber(latestForwardPacket.speed, false);
        }

        static unsigned long lastReverseSend = 0;
        const unsigned long now = millis();
        if (now - lastReverseSend >= REVERSE_PACKET_INTERVAL) {
            lastReverseSend = now;
            const IndicatorState state = debugMode ? INDICATOR_OFF : lastIndicatorState;
            ReversePacket packet = {
                state, headlights.toggleState(), getBrakePedalState(), true, ignitionState, starterState
            };

            sendPacket(Serial, 2, reinterpret_cast<uint8_t *>(&packet), sizeof(packet));
        }
    }
