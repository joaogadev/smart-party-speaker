#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

const int PIN_SENSOR = 3;
const int NUM_LEITURAS = 20;


// ======================================================
// MAC DA CAIXA
// ======================================================
//
// ESP32 principal / caixa:
//
// 8C:94:DF:70:D7:54

uint8_t macCaixa[] = {
  0x8C, 0x94, 0xDF,
  0x70, 0xD7, 0x54
};


// ======================================================
// DADOS ENVIADOS PELO ESP-NOW
// ======================================================

struct DadosCopo {

  int raw;
  int adc;
  int nivel;
};

DadosCopo dados;


// ======================================================
// PONTOS DE CALIBRAÇÃO
// ======================================================
//
// NÃO ALTEREI SUA CALIBRAÇÃO.
//
// Vazio:
// RAW aproximadamente 0
//
// Metade:
// RAW aproximadamente 2100
//
// Cheio:
// RAW aproximadamente 2460

const int RAW_0 = 0;
const int RAW_50 = 2100;
const int RAW_100 = 2460;


// ======================================================
// LEITURA
// ======================================================
//
// NÃO ALTERADO.

int lerSensor() {

  long soma = 0;

  for (int i = 0; i < NUM_LEITURAS; i++) {

    soma += analogRead(PIN_SENSOR);

    delay(5);
  }

  return soma / NUM_LEITURAS;
}


// ======================================================
// CALIBRAÇÃO PARA 12 BITS
// ======================================================
//
// NÃO ALTERADO.

int calibrarADC(int raw) {

  // ---------------------------
  // VAZIO ATÉ METADE
  // ---------------------------

  if (raw <= RAW_50) {

    int adc = map(
      raw,
      RAW_0,
      RAW_50,
      0,
      2048
    );

    return constrain(
      adc,
      0,
      2048
    );
  }


  // ---------------------------
  // METADE ATÉ CHEIO
  // ---------------------------

  int adc = map(
    raw,
    RAW_50,
    RAW_100,
    2048,
    4095
  );


  return constrain(
    adc,
    2048,
    4095
  );
}


// ======================================================
// ADC -> PORCENTAGEM
// ======================================================
//
// NÃO ALTERADO.

int calcularNivel(int adc) {

  int nivel = map(
    adc,
    0,
    4095,
    0,
    100
  );


  return constrain(
    nivel,
    0,
    100
  );
}


// ======================================================
// CONFIGURA ESP-NOW
// ======================================================

void configurarEspNow() {

  WiFi.mode(WIFI_STA);


  Serial.print("MAC do copo: ");
  Serial.println(WiFi.macAddress());


  if (esp_now_init() != ESP_OK) {

    Serial.println(
      "ERRO ao iniciar ESP-NOW"
    );

    return;
  }


  esp_now_peer_info_t peerInfo = {};


  memcpy(
    peerInfo.peer_addr,
    macCaixa,
    6
  );


  peerInfo.channel = 0;
  peerInfo.encrypt = false;


  if (esp_now_add_peer(&peerInfo) != ESP_OK) {

    Serial.println(
      "ERRO ao adicionar caixa"
    );

    return;
  }


  Serial.println(
    "ESP-NOW configurado!"
  );
}


// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);

  delay(1000);


  // ====================================================
  // ADC
  // ====================================================
  //
  // NÃO ALTERADO.

  analogReadResolution(12);


  // NÃO ALTERADO.
  //
  // Continua exatamente em 11 dB.

  analogSetPinAttenuation(
    PIN_SENSOR,
    ADC_11db
  );


  Serial.println();
  Serial.println("============================");
  Serial.println(" CALIBRACAO POR FAIXAS");
  Serial.println("============================");


  // Única adição:
  // configuração da comunicação ESP-NOW.

  configurarEspNow();


  Serial.println();
}


// ======================================================
// LOOP
// ======================================================

void loop() {

  // ====================================================
  // 1. LEITURA
  // ====================================================
  //
  // EXATAMENTE COMO ESTAVA.

  int raw = lerSensor();


  // ====================================================
  // 2. CALIBRAÇÃO
  // ====================================================
  //
  // EXATAMENTE COMO ESTAVA.

  int adcCalibrado = calibrarADC(raw);


  // ====================================================
  // 3. PORCENTAGEM
  // ====================================================
  //
  // EXATAMENTE COMO ESTAVA.

  int nivel = calcularNivel(
    adcCalibrado
  );


  // ====================================================
  // 4. PREPARA PACOTE ESP-NOW
  // ====================================================

  dados.raw = raw;
  dados.adc = adcCalibrado;
  dados.nivel = nivel;


  // ====================================================
  // 5. ENVIA PARA A CAIXA
  // ====================================================

  esp_err_t resultado = esp_now_send(
    macCaixa,
    (uint8_t *) &dados,
    sizeof(dados)
  );


  // ====================================================
  // 6. SERIAL
  // ====================================================
  //
  // Mantive os mesmos valores que você já estava
  // visualizando.

  Serial.print("RAW: ");
  Serial.print(raw);


  Serial.print(" | ADC 12 bits: ");
  Serial.print(adcCalibrado);


  Serial.print(" | Nivel: ");
  Serial.print(nivel);


  Serial.print("% | ESP-NOW: ");


  if (resultado == ESP_OK) {

    Serial.println("ENVIADO");

  } else {

    Serial.println("ERRO");
  }


  // ====================================================
  // DELAY ORIGINAL
  // ====================================================
  //
  // NÃO ALTERADO.

  delay(1000);
}