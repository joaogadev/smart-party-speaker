#include "EspNowManager.h"

EspNowManager espNow;

void dadosRecebidos(const uint8_t *mac, const uint8_t *dados, int tamanho) {
    Serial.print("Pacote recebido. Tamanho: ");

    Serial.println(tamanho);
}

void setup() {
    Serial.begin(115200);

    delay(1000);

    espNow.definirCallback(
        dadosRecebidos
    );

    if (espNow.iniciar()) {
        Serial.println("Caixa pronta para receber.");
    }
}

void loop() {
}