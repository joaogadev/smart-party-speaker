// Classe para mostrar quais funções o esp now possui, como enviar e receber dados, adicionar peers e etc.

#ifndef ESP_NOW_MANAGER_H
#define ESP_NOW_MANAGER_H

#include <Arduino.h>
#include <esp_now.h>
#include <esp_arduino_version.h>


class EspNowManager {

public:
    /*
     * Callback utilizado pelo restante do projeto.
     *
     * Independente da versão do ESP-NOW,
     * quem utilizar o EspNowManager receberá:
     *
     * - MAC de quem enviou
     * - dados recebidos
     * - tamanho dos dados
     */
    using ReceiveCallback = void (*)(
        const uint8_t *mac,
        const uint8_t *dados,
        int tamanho
    );

    /*
     * Inicializa a comunicação ESP-NOW.
     */
    bool iniciar();

    /*
     * Define qual função do projeto será chamada
     * quando chegar uma mensagem.
     */
    void definirCallback(
        ReceiveCallback callback
    );

private:
    /*
     * Callback definido pelo usuário.
     */
    static ReceiveCallback callbackUsuario;

    /*
     * Arduino-ESP32 3.x utiliza a nova API.
     */
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    static void aoReceberDados(
        const esp_now_recv_info_t *info,
        const uint8_t *dados,
        int tamanho
    );

#else
    /*
     * Arduino-ESP32 2.x utiliza a API antiga.
     */
    static void aoReceberDados(
        const uint8_t *mac,
        const uint8_t *dados,
        int tamanho
    );

#endif
};

#endif