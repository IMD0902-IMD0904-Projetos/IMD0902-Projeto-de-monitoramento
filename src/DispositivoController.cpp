#include <Arduino.h>
#include <SPI.h>

#include "DispositivoController.h";

//=== Métodos do Game Loop
void DispositivoController::inicializar() {
    Serial.begin(115200);
    SPI.begin();
    rfid.inicializar();
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BOTAO_PIN, INPUT);
}

void DispositivoController::processarEventos() {
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        distancia = lerDistancia();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        distancia = lerDistancia();
        // FIXME Corrigir para implementação real depois
        rfid.ler();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        lerBotao();
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        // TODO A definir
    }
}

void DispositivoController::atualizar() {
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        if(distancia <= DISTANCIA_TRIGGER) {
            estadoAnterior = estadoAtual;
            estadoAtual = EstadoEstacao::OCUPADA_SEM_REGISTRO;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        if(distancia <= DISTANCIA_TRIGGER) {
            // TODO Se o rfid for lido com sucesso, atualiza o estado para ocupada
            // estadoAnterior = estadoAtual;
            // estadoAtual = EstadoEstacao::OCUPADA;
        }
        else {
            estadoAnterior = estadoAtual;
            estadoAtual = EstadoEstacao::DISPONIVEL;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        // TODO Se tiver ocupada e apertar o botão no modo sair, tornar disponível
        // estadoAnterior = estadoAtual;
        // estadoAtual = EstadoEstacao::DISPONIVEL;
        lerBotao();
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        // TODO A definir
    }
}

void DispositivoController::renderizar() const {
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Disponível!");
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Ocupada sem registro!");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Ocupada sem registro!");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        Serial.println(">>> ---------------------------- <<<");
        // TODO Trocar pelo identificador atual de fato
        Serial.println(">>> Identificador: '0x48 0xFA 0x02'");
        Serial.println(">>> Ocupada!");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        // TODO A definir
    }
}


///=== Métodos de processar eventos

float DispositivoController::lerDistancia() {
    long duracaoPulso = pulseIn(ECHO_PIN, HIGH);
    return (duracaoPulso * VELOCIDADE_DO_SOM) / 2;
}

// TODO Melhorar lógica para retornar os segundos cujo o botão ficou pressionado
/**
 * Tempo pressionado < 3 = Modo sair da estação
 * Tempo pressionado >= 3 = Modo manutenção
 * Tempo pressionado >= 7 = Modo configuração (avaliar necessidade)
 */
int DispositivoController::lerBotao() {
    int touchValor = 0;
    touchValor = digitalRead(BOTAO_PIN);
    if(touchValor) {
        Serial.println("Botão pressionado");
    }
    return touchValor;
}

//=== Métodos utilitários
void DispositivoController::executarPulso() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}