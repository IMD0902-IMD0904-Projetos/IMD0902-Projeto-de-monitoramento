#include <Arduino.h>
#include "Botao.h"

void Botao::inicializar() {
    pinMode(BOTAO_PIN, INPUT);
}

void Botao::lerBotao() {
    estadoAtual = static_cast<EstadoBotao>(digitalRead(BOTAO_PIN));
    if(estadoAtual == EstadoBotao::PRESSIONADO && estadoAnterior == EstadoBotao::NAO_PRESSIONADO) {
        inicioTempoPressao = millis();
    }
    if(estadoAtual == EstadoBotao::NAO_PRESSIONADO && estadoAnterior == EstadoBotao::PRESSIONADO) {
        unsigned long duracao = millis() - inicioTempoPressao;
        Serial.println("Tempo de pressão (ms): " + String(duracao));
        this->definirModo(duracao);
    }
    estadoAnterior = estadoAtual;
}

void Botao::resetarBotao() {
    this->modo = ModoBotao::DESLIGADO;
}

void Botao::definirModo(const unsigned long& duracao) {
    if(duracao >= 3000 && duracao < 7000) {
        this->modo = ModoBotao::SOLICITAR_MANUTENCAO;
    }
    else if(duracao >= 7000) {
        this->modo = ModoBotao::CONFIGURACAO;
    }
}

Botao::ModoBotao Botao::obterModo() const {
    return this->modo;
}