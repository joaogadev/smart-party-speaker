
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_arduino_version.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// ======================================================
// CONFIGURACAO BLUETOOTH BLE
// ======================================================

#define NOME_BLE "SMART_PARTY_SPEAKER"

#define SERVICE_UUID \
  "12345678-1234-1234-1234-123456789abc"

#define NIVEL_UUID \
  "12345678-1234-1234-1234-123456789abd"

// ======================================================
// ESTRUTURA RECEBIDA DO COPO
// DEVE SER IDENTICA AO CODIGO DO ESP32-C3
// ======================================================

struct DadosCopo {
  int raw;
  int adc;
  int nivel;
};

// ======================================================
// VARIAVEIS DO ESP-NOW
// ======================================================

DadosCopo dadosRecebidos = {};

volatile bool novoPacote = false;

portMUX_TYPE mutexDados =
    portMUX_INITIALIZER_UNLOCKED;

unsigned long ultimoPacote = 0;
unsigned long ultimaMensagem = 0;
unsigned long totalPacotes = 0;

// ======================================================
// VARIAVEIS DO BLE
// ======================================================

BLEServer *servidorBLE = nullptr;
BLECharacteristic *caracteristicaNivel = nullptr;

volatile bool celularConectado = false;
volatile bool reiniciarAnuncio = false;

// ======================================================
// CALLBACKS BLE
// ======================================================

class EventosBLE : public BLEServerCallbacks {

  void onConnect(BLEServer *servidor) override {
    celularConectado = true;
  }

  void onDisconnect(BLEServer *servidor) override {
    celularConectado = false;
    reiniciarAnuncio = true;
  }
};

// ======================================================
// PROCESSAMENTO DOS DADOS ESP-NOW
// ======================================================

void processarDados(
  const uint8_t *dados,
  int tamanho
) {

  if (tamanho != sizeof(DadosCopo)) {
    return;
  }

  portENTER_CRITICAL(&mutexDados);

  memcpy(
    &dadosRecebidos,
    dados,
    sizeof(DadosCopo)
  );

  novoPacote = true;

  portEXIT_CRITICAL(&mutexDados);
}

// ======================================================
// CALLBACK ESP-NOW
// COMPATIVEL COM ARDUINO-ESP32 2.x E 3.x
// ======================================================

#if ESP_ARDUINO_VERSION_MAJOR >= 3

void aoReceberDados(
  const esp_now_recv_info_t *info,
  const uint8_t *dados,
  int tamanho
) {
  processarDados(dados, tamanho);
}

#else

void aoReceberDados(
  const uint8_t *mac,
  const uint8_t *dados,
  int tamanho
) {
  processarDados(dados, tamanho);
}

#endif

// ======================================================
// CONFIGURACAO ESP-NOW
// ======================================================

bool configurarEspNow() {

  WiFi.mode(WIFI_STA);

  Serial.print("[ESP-NOW] MAC da caixa: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {

    Serial.println(
      "[ERRO] Falha ao iniciar ESP-NOW!"
    );

    return false;
  }

  if (
    esp_now_register_recv_cb(aoReceberDados)
    != ESP_OK
  ) {

    Serial.println(
      "[ERRO] Falha no callback ESP-NOW!"
    );

    return false;
  }

  Serial.println("[ESP-NOW] Inicializado!");

  return true;
}

// ======================================================
// CONFIGURACAO BLUETOOTH BLE
// ======================================================

void configurarBLE() {

  Serial.println("[BLE] Inicializando...");

  BLEDevice::init(NOME_BLE);

  servidorBLE = BLEDevice::createServer();

  servidorBLE->setCallbacks(
    new EventosBLE()
  );

  BLEService *servico =
      servidorBLE->createService(SERVICE_UUID);

  caracteristicaNivel =
      servico->createCharacteristic(
        NIVEL_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
      );

  caracteristicaNivel->addDescriptor(
    new BLE2902()
  );

  // Nivel inicial
  caracteristicaNivel->setValue("0");

  servico->start();

  BLEAdvertising *anuncio =
      BLEDevice::getAdvertising();

  anuncio->addServiceUUID(SERVICE_UUID);

  anuncio->setScanResponse(true);

  BLEDevice::startAdvertising();

  Serial.println("[BLE] Inicializado!");

  Serial.print("[BLE] Nome: ");
  Serial.println(NOME_BLE);

  Serial.println(
    "[BLE] Aguardando Android..."
  );
}

// ======================================================
// ENVIA O NIVEL PARA O CELULAR
// ======================================================

void enviarNivelBLE(int nivel) {

  if (caracteristicaNivel == nullptr) {
    return;
  }

  nivel = constrain(nivel, 0, 100);

  String texto = String(nivel);

  // Atualiza o valor disponivel para leitura
  caracteristicaNivel->setValue(
    texto.c_str()
  );

  // Notifica quando existe celular conectado
  if (celularConectado) {

    caracteristicaNivel->notify();

    Serial.print("[BLE] Nivel enviado: ");
    Serial.print(nivel);
    Serial.println("%");
  }
}

// ======================================================
// SETUP
// ======================================================

void setup() {

  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("============================");
  Serial.println("     SMART PARTY SPEAKER");
  Serial.println("        ESP-NOW + BLE");
  Serial.println("============================");

  // 1. Inicializa ESP-NOW
  if (!configurarEspNow()) {

    Serial.println(
      "[ERRO] ESP-NOW indisponivel!"
    );

    return;
  }

  // 2. Inicializa BLE
  configurarBLE();

  Serial.println();
  Serial.println("Sistema inicializado!");
  Serial.println("Aguardando dados do copo...");
}

// ======================================================
// LOOP
// ======================================================

void loop() {

  DadosCopo copia = {};
  bool recebeu = false;

  // ====================================================
  // COPIA SEGURA DOS DADOS
  // ====================================================

  portENTER_CRITICAL(&mutexDados);

  if (novoPacote) {

    copia = dadosRecebidos;

    novoPacote = false;

    recebeu = true;
  }

  portEXIT_CRITICAL(&mutexDados);

  // ====================================================
  // PROCESSAMENTO DO NIVEL RECEBIDO
  // ====================================================

  if (recebeu) {

    ultimoPacote = millis();
    totalPacotes++;

    int nivel = constrain(
      copia.nivel,
      0,
      100
    );

    Serial.print("[COPO] RAW: ");
    Serial.print(copia.raw);

    Serial.print(" | ADC: ");
    Serial.print(copia.adc);

    Serial.print(" | Nivel: ");
    Serial.print(nivel);
    Serial.println("%");

    // Transmite o nivel por BLE
    enviarNivelBLE(nivel);
  }

  // ====================================================
  // REINICIA PUBLICIDADE BLE APOS DESCONEXAO
  // ====================================================

  if (reiniciarAnuncio) {

    reiniciarAnuncio = false;

    delay(100);

    BLEDevice::startAdvertising();

    Serial.println(
      "[BLE] Aguardando nova conexao..."
    );
  }

  // ====================================================
  // MONITORAMENTO
  // ====================================================

  if (millis() - ultimaMensagem >= 3000) {

    ultimaMensagem = millis();

    Serial.println();
    Serial.println("========== STATUS ==========");

    Serial.print("Pacotes ESP-NOW: ");
    Serial.println(totalPacotes);

    Serial.print("Android BLE: ");

    if (celularConectado) {
      Serial.println("CONECTADO");
    } else {
      Serial.println("DESCONECTADO");
    }

    Serial.print("Copo: ");

    if (ultimoPacote == 0) {

      Serial.println("SEM DADOS");

    } else if (
      millis() - ultimoPacote > 5000
    ) {

      Serial.println("SEM DADOS RECENTES");

    } else {

      Serial.println("ATIVO");
    }

    Serial.println("============================");
    Serial.println();
  }

  delay(10);
}