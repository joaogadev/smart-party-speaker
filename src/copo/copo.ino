#include "EspNowManager.h"

EspNowManager espNow;

// MAC da caixa
const uint8_t MAC_CAIXA[] = {
    0x8C,
    0x94,
    0xDF,
    0x70,
    0xD7,
    0x54
};

void setup() {
    Serial.begin(115200);

    delay(1000);

    if (espNow.iniciar(MAC_CAIXA)) {
        Serial.println(
            "Comunicacao ESP-NOW pronta."
        );
    }
}

void loop() {

}