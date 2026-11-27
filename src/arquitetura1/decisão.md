# Arquitetura 1
### Arquitetura que bota o celular como receptor de informações do esp32 convencional e envia para a caixa os valores das porcentagens após a conversão dos valores recebidos pelo sensor de volume d'água. É a arquitetura que seguirá com o projeto
```mermaid
flowchart LR
    S[Sensor de nível] -->|Sinal analógico| C[ESP32-C3 no copo]
    C -->|ESP-NOW: RAW, ADC, nível| E[ESP32 convencional]
    E -->|BLE GATT: nível em texto| A[Aplicativo Android]
    F[Arquivo de áudio local] --> A
    A -->|Bluetooth A2DP: áudio| J[JBL Flip 7]
```