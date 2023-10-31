#include "Conexao.h"
#include <WiFi.h>

void Conexao::inicializar() {
    pinMode(LED_CONEXAO_PIN, OUTPUT);
    
    Serial.println("Procurando redes Wi-Fi disponíveis...");
    int numRedes = WiFi.scanNetworks();

    if (numRedes == 0) {
        Serial.println("Nenhuma rede Wi-Fi encontrada.");
    } else {
    Serial.print(numRedes);
    Serial.println(" redes Wi-Fi encontradas:");

    for (int i = 0; i < numRedes; i++) {
      Serial.print("Rede ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID(i));
      Serial.println("-------------");
    }
  }
}

void Conexao::conectar() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long tempoInicio = millis();   

    while(WiFi.status() != WL_CONNECTED && (millis() - tempoInicio < WIFI_TIMEOUT)) {
        delay(100);
    }

    if(this->estaConectado()) {
        Serial.println("Conectado à rede!");
        digitalWrite(LED_CONEXAO_PIN, HIGH);
    }
}

bool Conexao::estaConectado() {
    if(static_cast<int>(WiFi.status()) >= 4) {
        Serial.println("Wifi desconectado!");
        digitalWrite(LED_CONEXAO_PIN, LOW);
    }
    return WiFi.status() == WL_CONNECTED;
}