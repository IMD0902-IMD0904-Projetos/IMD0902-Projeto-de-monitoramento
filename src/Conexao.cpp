#include "Conexao.h"
#include <WiFi.h>

void Conexao::inicializar() {
    WiFi.mode(WIFI_STA);
    pinMode(LED_CONEXAO_PIN, OUTPUT);
}

void Conexao::conectar() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long tempoInicio = millis();   

    while(WiFi.status() != WL_CONNECTED && (millis() - tempoInicio < WIFI_TIMEOUT)) {
        delay(100);
    }

    if(this->estaConectado()) {
        digitalWrite(LED_CONEXAO_PIN, HIGH);
    }
}

bool Conexao::estaConectado() {
    return WiFi.status() == WL_CONNECTED;
}