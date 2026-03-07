#include <Arduino.h>
#include <SerialPacketFunctions.h>


void handlePacket(uint8_t type, uint8_t *data, uint8_t len) {
    Serial.print("Received type: "); Serial.println(type);
    for (uint8_t i = 0; i < len; i++) {
        Serial.print(data[i]); Serial.print(" ");
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    updatePacket(Serial, handlePacket);

    static unsigned long last = 0;
    if (millis() - last > 2000) {
        last = millis();
        uint8_t payload[3] = {1,2,3};
        sendPacket(Serial, 1, payload, 3);
    }
}