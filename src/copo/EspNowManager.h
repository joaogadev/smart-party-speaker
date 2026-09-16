// Classe para mostrar quais funções o esp now possui, como enviar e receber dados, adicionar peers e etc.

#ifndef ESP_NOW_MANAGER_H
#define ESP_NOW_MANAGER_H

#include <Arduino.h>

class EspNowManager {

public:
    // Inicializa ESP-NOW e registra
    // o ESP32 que receberá as mensagens.
    bool iniciar(const uint8_t *macDestino);

    // Envia qualquer estrutura/dado
    // para o ESP32 configurado.
    bool enviar(
        const void *dados,
        size_t tamanho
    );


private:
    uint8_t macDestino[6];

    bool iniciado = false;
};

#endif