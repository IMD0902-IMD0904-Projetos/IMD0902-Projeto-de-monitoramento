#include <Arduino.h>
#include <SPI.h>
#include <cstring>
#include "DispositivoController.h"

//=== Métodos do Game Loop
void DispositivoController::inicializar() {
    Serial.begin(115200);
    SPI.begin();
    rfid.inicializar();
    botao.inicializar();
    conexao.inicializar();
    produtor.inicializar();
    conexao.conectar();
    if(conexao.estaConectado()) {
        produtor.conectar();
    }
    *alunoAtual = Produtor::Aluno(0, "", "", "");
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_VERDE_PIN, OUTPUT);
    pinMode(LED_AMARELA_PIN, OUTPUT);
    pinMode(LED_AZUL_PIN, OUTPUT);
    pinMode(LED_VERMELHA_PIN, OUTPUT);
}

void DispositivoController::processarEventos() {
    botao.resetarBotao();
    botao.lerBotao();
    // Adota política de reconexão em caso de instabilidade
    if(!conexao.estaConectado()) {
        conexao.conectar();
    }
    if(conexao.estaConectado() && !produtor.estaConectado()) {
        produtor.conectar();
    }
    // Toma ações a depender do estado
    if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        distancia = lerDistancia();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        distancia = lerDistancia();
        rfid.ler();
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_COM_REGISTRO) {
        rfid.ler();
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        rfid.ler();
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
                *alunoAtual = obterAlunoPorTag(rfid.obterIdentificador().c_str());
                if(this->alunoAtual->id != 0) {
                    estadoAtual = EstadoEstacao::OCUPADA_COM_REGISTRO;
                    Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, alunoAtual);
                    produtor.publicarMensagemEntrada(&msgAcesso);
                }
                rfid.limparIdentificador();
            }
        }
        else {
            estadoAtual = EstadoEstacao::DISPONIVEL;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_COM_REGISTRO) {
        if(!rfid.obterIdentificador().isEmpty()) {
            // Só desloga o usuário caso a tag aproximada seja igual à tag do ocupante  
            if(std::strcmp(this->alunoAtual->tag, rfid.obterIdentificador().c_str()) == 0) {
                estadoAtual = EstadoEstacao::DISPONIVEL;
                Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, alunoAtual);
                produtor.publicarMensagemSaida(&msgAcesso);
                *alunoAtual = Produtor::Aluno(0, "", "", "");
            }

            rfid.limparIdentificador();
        }
        else if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
            estadoAtual = EstadoEstacao::EM_MANUTENCAO;
            Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, alunoAtual);
            produtor.publicarMensagemSaida(&msgAcesso);
            *alunoAtual = Produtor::Aluno(0, "", "", "");
            Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            produtor.publicarMensagemAlteracaoEstado(&msg);
            rfid.limparIdentificador();
        }
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            produtor.publicarMensagemAlteracaoEstado(&msg);
        }
    }
}

void DispositivoController::renderizar() const {
    if(estadoAtual == EstadoEstacao::DISPONIVEL && estadoAnterior != estadoAtual) {
        this->ligarLedVerde();
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
        Serial.println(">>> Estado: OCUPADA_SEM_REGISTRO  <<<");
        Serial.println(">>> ---------------------------- <<<");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_COM_REGISTRO && estadoAnterior != estadoAtual) {
        this->ligarLedAzul();
        Serial.println(">>> ---------------------------- <<<");
        Serial.println(">>> Sistema de Presença - IMD0902<<<");
        Serial.println(">>>                              <<<");
        Serial.println(">>> Identificador: " + rfid.obterIdentificador());
        Serial.println(">>> Estado: OCUPADA_COM_REGISTRO <<<");
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
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_AZUL_PIN, HIGH);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedAmarela() const {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, HIGH);
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedVerde() const {
    digitalWrite(LED_VERDE_PIN, HIGH);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, LOW);
}

void DispositivoController::ligarLedVermelha() const {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_AMARELA_PIN, LOW);
    digitalWrite(LED_AZUL_PIN, LOW);
    digitalWrite(LED_VERMELHA_PIN, HIGH);
}

Produtor::Aluno DispositivoController::obterAlunoPorTag(const char* tag) {
    for (int i = 0; i < 3; i++) {
        if (std::strcmp(listaAlunos[i].tag, tag) == 0) {
            return listaAlunos[i];
        }
    }
    return Produtor::Aluno(0, "", "", "");
}