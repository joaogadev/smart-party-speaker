#include "EspNowManager.h"

#include <WiFi.h>
#include <esp_now.h>


// Inicialmente nenhuma função externa
// está registrada para receber os dados.
EspNowManager::ReceiveCallback
EspNowManager::callbackUsuario = nullptr;

bool EspNowManager::iniciar() {

    // ESP-NOW utiliza a interface Wi-Fi.
    WiFi.mode(WIFI_STA);

    // Inicializa ESP-NOW.
    if (esp_now_init() != ESP_OK) {
        Serial.println(
            "Erro ao iniciar ESP-NOW"
        );

        return false;
    }

    // Registra nossa função interna
    // para receber mensagens ESP-NOW.
    if (esp_now_register_recv_cb(aoReceberDados) != ESP_OK) {
        Serial.println("Erro ao registrar callback");

        return false;
    }

    Serial.println("ESP-NOW inicializado.");

    return true;
}

void EspNowManager::definirCallback( ReceiveCallback callback) {
    callbackUsuario = callback;
}

/*
 * =========================================================
 * Arduino ESP32 3.x
 * =========================================================
 */

#if ESP_ARDUINO_VERSION_MAJOR >= 3

void EspNowManager::aoReceberDados(const esp_now_recv_info_t *info, const uint8_t *dados, int tamanho) {
    // Nenhum callback externo foi configurado.
    if (callbackUsuario == nullptr) {
        return;
    }

    // Passa os dados recebidos para
    // o restante da aplicação.
    callbackUsuario(
        info->src_addr,
        dados,
        tamanho
    );
}

/*
 * =========================================================
 * Arduino ESP32 2.x
 * =========================================================
 */

#else


void EspNowManager::aoReceberDados(
    const uint8_t *mac,
    const uint8_t *dados,
    int tamanho
) {

    // Nenhum callback externo foi configurado.
    if (callbackUsuario == nullptr) {

        return;
    }


    // Na API antiga o MAC já vem
    // diretamente como parâmetro.
    callbackUsuario(
        mac,
        dados,
        tamanho
    );
}


#endif