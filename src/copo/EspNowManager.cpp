// Implementação do ESP-NOW, adicionando outro ESP como peer e enviando dados.

#include "EspNowManager.h"

#include <WiFi.h>
#include <esp_now.h>

#include <cstring>


bool EspNowManager::iniciar(const uint8_t *mac) {
    // ESP-NOW utiliza a interface Wi-Fi
    WiFi.mode(WIFI_STA);

    // Inicializa ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao iniciar ESP-NOW");

        return false;
    }

    // Salva o MAC do destinatário
    memcpy(macDestino, mac,6);

    // Configuração do peer
    esp_now_peer_info_t peerInfo = {};

    memcpy(
        peerInfo.peer_addr,
        macDestino,
        6
    );

    peerInfo.channel = 0;

    peerInfo.encrypt = false;


    // Registra o peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Erro ao adicionar peer");

        return false;
    }

    iniciado = true;

    Serial.println("ESP-NOW inicializado.");

    return true;
}


bool EspNowManager::enviar(const void *dados, size_t tamanho) {
    if (!iniciado) {
        return false;
    }


    esp_err_t resultado =esp_now_send(
            macDestino,
            reinterpret_cast<const uint8_t *>(dados),
            tamanho
        );


    return resultado == ESP_OK;
}