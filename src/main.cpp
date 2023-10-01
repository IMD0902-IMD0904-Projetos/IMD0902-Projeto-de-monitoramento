#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

enum EstadoEstacaoDeTrabalho {
    DISPONIVEL,
    OCUPADA_SEM_REGISTRO,
    OCUPADA,
    EM_MANUTENCAO
};

// Em cm/uS
const float VELOCIDADE_DO_SOM = 0.034;
const float DISTANCIA_TRIGGER = 40.0;
const int TRIGGER_PIN = 13;
const int ECHO_PIN = 12;
const int BOTAO_PIN = 14;

const int SS_PIN = 5;
const int RST_PIN = 15;
MFRC522 rfid(SS_PIN, RST_PIN);
EstadoEstacaoDeTrabalho estado = EstadoEstacaoDeTrabalho::DISPONIVEL;

void executarPulso() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}

float obterDistancia() {
    long duracaoPulso = pulseIn(ECHO_PIN, HIGH);
    return (duracaoPulso * VELOCIDADE_DO_SOM) / 2;
}

void setup() {
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BOTAO_PIN, INPUT);
    Serial.println("Projeto de monitoramento iniciado!");
}


void processarEventos() {
  executarPulso();
  float distanciaAtual = obterDistancia();
  if(distanciaAtual <= DISTANCIA_TRIGGER) {
    if(estado != EstadoEstacaoDeTrabalho::OCUPADA_SEM_REGISTRO) {
        estado = EstadoEstacaoDeTrabalho::OCUPADA_SEM_REGISTRO;
        Serial.println(">>> Distância: " + String(distanciaAtual));
        Serial.println(">>> Ocupada sem registro!");
    }
  }
  else {
    if(estado != EstadoEstacaoDeTrabalho::DISPONIVEL) {
        estado = EstadoEstacaoDeTrabalho::DISPONIVEL;
        Serial.println(">>> Distância: " + String(distanciaAtual));
        Serial.println(">>> Disponivel!");
    }
  }
}

void loop() {
  processarEventos();
  int touchValor = 0;
  touchValor = digitalRead(BOTAO_PIN);
  if(touchValor) {
    Serial.println("Botão pressionado");
  }

  if(estado == EstadoEstacaoDeTrabalho::OCUPADA_SEM_REGISTRO) {
    if (rfid.PICC_IsNewCardPresent()) { 
        if (rfid.PICC_ReadCardSerial()) {
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
            Serial.print("RFID/NFC Tag Type: ");
            Serial.println(rfid.PICC_GetTypeName(piccType));
            Serial.print("UID:");
            for (int i = 0; i < rfid.uid.size; i++) {
                Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(rfid.uid.uidByte[i], HEX);
            }
            Serial.println();

            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
        }
    }
  }
 

  delay(800);
}



