#include <Arduino.h>
#include <SPI.h>
#include "DispositivoController.h"

//=== Métodos do Game Loop
void DispositivoController::inicializar() {
    Serial.begin(115200);
    SPI.begin();
    rfid.inicializar();
    botao.inicializar();
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_AZUL_PIN, OUTPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    pinMode(LED_AMARELA_PIN, OUTPUT);
    pinMode(LED_VERMELHA_PIN, OUTPUT);
}

void DispositivoController::processarEventos() {
    botao.resetarBotao();
    botao.lerBotao();
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        distancia = lerDistancia();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        distancia = lerDistancia();
        rfid.ler();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        // TODO A definir
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        // TODO A definir
    }
}

void DispositivoController::atualizar() {
    estadoAnterior = estadoAnterior != EstadoEstacao::DESLIGADO ? estadoAtual : estadoAnterior;
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        if(distancia <= DISTANCIA_TRIGGER) {
            estadoAtual = EstadoEstacao::OCUPADA_SEM_REGISTRO;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        if(distancia <= DISTANCIA_TRIGGER) {
            if(!rfid.obterIdentificador().isEmpty()) {
                estadoAtual = EstadoEstacao::OCUPADA;
            }
        }
        else {
            estadoAtual = EstadoEstacao::DISPONIVEL;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        if(botao.obterModo() == Botao::ModoBotao::SAIDA) {
            // TODO Enviar mensagem por MQTT dizendo que o usuário deslogou
            estadoAtual = EstadoEstacao::DISPONIVEL;
            rfid.limparIdentificador();
        }
        else if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
            // TODO Enviar mensagem por MQTT dizendo que o usuário deslogou e solicitou manutenção
            estadoAtual = EstadoEstacao::EM_MANUTENCAO;
            rfid.limparIdentificador();
        }
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
            // TODO Enviar mensagem por MQTT dizendo que a estação está disponível novamente
            estadoAtual = EstadoEstacao::DISPONIVEL;
        }
    }
}

void DispositivoController::renderizar() const {
    if(estadoAtual == EstadoEstacao::DISPONIVEL && estadoAnterior != estadoAtual) {
        this->ligarLedAzul();
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Sistema de Presença - IMD0902<<<");
        Serial.println(">>>                              <<<");
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Estado: DISPONIVEL           <<<");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO && estadoAnterior != estadoAtual) {
        this->ligarLedAmarela();
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Sistema de Presença - IMD0902<<<");
        Serial.println(">>>                              <<<");
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Estado: AGUARDANDO_REGISTRO  <<<");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA && estadoAnterior != estadoAtual) {
        this->ligarLedVerde();
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Sistema de Presença - IMD0902<<<");
        Serial.println(">>>                              <<<");
        Serial.println(">>> Identificador: " + rfid.obterIdentificador());
        Serial.println(">>> Estado: OCUPADA              <<<");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO && estadoAnterior != estadoAtual) {
        this->ligarLedVermelha();
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Sistema de Presença - IMD0902<<<");
        Serial.println(">>>                              <<<");
        Serial.println(">>> A máquina está indisponível. <<<");
        Serial.println(">>> Estado: EM_MANUTENCAO        <<<");
        Serial.println(">>> ---------------------------- <<<");
    }
}


///=== Métodos de processar eventos

float DispositivoController::lerDistancia() {
    this->executarPulso();
    long duracaoPulso = pulseIn(ECHO_PIN, HIGH);
    return (duracaoPulso * VELOCIDADE_DO_SOM) / 2;
}

//=== Métodos utilitários
void DispositivoController::executarPulso() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}

void DispositivoController::ligarLedAzul() const {
    digitalWrite(LED_AZUL_PIN, HIGH);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedAmarela() const {
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, HIGH);
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedVerde() const {
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_VERDE_PIN, HIGH);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedVermelha() const {
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, HIGH);
}