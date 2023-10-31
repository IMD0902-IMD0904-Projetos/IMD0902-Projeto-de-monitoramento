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
    distancia = lerDistancia();
    rfid.ler();
    // Adota política de reconexão em caso de instabilidade
    if(!conexao.estaConectado()) {
        Serial.println("Não conectado à rede. Conectando...");
        conexao.conectar();
    }
    if(conexao.estaConectado() && !produtor.estaConectado()) {
        Serial.println("Produtor não conectado ao broker.");
        produtor.conectar();
    }
}

void DispositivoController::atualizar() {
    estadoAnterior = estadoAtual;
    if (estadoAtual == EstadoEstacao::DESLIGADO) {
        estadoAtual = EstadoEstacao::DISPONIVEL;
        Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
        bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
        if(publicado) {
            Serial.println("Mensagem de alteração de estado publicada!");
        }
    }
    else if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        if(distancia <= DISTANCIA_TRIGGER) {
            estadoAtual = EstadoEstacao::OCUPADA_SEM_REGISTRO;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO) {
        if(distancia <= DISTANCIA_TRIGGER) {
            if(!rfid.obterIdentificador().isEmpty()) {
                Aluno alunoAtual = obterAlunoPorTag(rfid.obterIdentificador());
                if(alunoAtual.id != 0) {
                    estadoAtual = EstadoEstacao::OCUPADA_COM_REGISTRO;
                    tagLogada = alunoAtual.tag;
                    Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
                    bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
                    if(publicado) {
                        Serial.println("Mensagem de alteração de estado publicada!");
                    }
                    Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, alunoAtual);
                    bool publicadoEntrada = produtor.publicarMensagemEntrada(msgAcesso);
                    if(publicadoEntrada) {
                        Serial.println("Mensagem de entrada publicada!");
                    }
                }
            }
        }
        else {
            estadoAtual = EstadoEstacao::DISPONIVEL;
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_COM_REGISTRO) {
        if(!rfid.obterIdentificador().isEmpty() && rfid.obterIdentificador().indexOf(tagLogada) == 0) {
            // Só desloga o usuário caso a tag aproximada seja igual à tag do ocupante  
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
            if(publicado) {
                Serial.println("Mensagem de alteração de estado publicada!");
            }
            Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, obterAlunoPorTag(tagLogada));
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada!");
            }
            tagLogada = "";
        }
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        if(!rfid.obterIdentificador().isEmpty() && rfid.obterIdentificador().indexOf(tagAdmin) == 0) {
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
            if(publicado) {
                Serial.println("Mensagem de alteração de estado publicada!");
            }
        }
    }
    else if(estadoAtual == EstadoEstacao::CONFIGURACAO) {
        if(!rfid.obterIdentificador().isEmpty()) {
            tagAdmin = rfid.obterIdentificador();
            estadoAtual = ultimoEstadoPreConfiguracao;
        }
    }
    
    if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
        estadoAtual = EstadoEstacao::EM_MANUTENCAO;
        Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
        bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
        if(publicado) {
            Serial.println("Mensagem de alteração de estado publicada com sucesso!");
        }
        if(!tagLogada.isEmpty()) {
            Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, obterAlunoPorTag(tagLogada));
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada com sucesso!");
            }
            tagLogada = "";
        }
        botao.resetarBotao();
    }
    else if(botao.obterModo() == Botao::ModoBotao::CONFIGURACAO) {
        ultimoEstadoPreConfiguracao = estadoAtual;
        estadoAtual = EstadoEstacao::CONFIGURACAO;
        botao.resetarBotao();
    }
    rfid.limparIdentificador();
}

void DispositivoController::renderizar() const {
    if(estadoAtual == EstadoEstacao::DISPONIVEL && estadoAnterior != estadoAtual) {
        this->ligarLedVerde();
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Estado: DISPONIVEL");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_SEM_REGISTRO && estadoAnterior != estadoAtual) {
        this->ligarLedAmarela();
        Serial.println(">>> Distância (cm): " + String(distancia));
        Serial.println(">>> Estado: OCUPADA_SEM_REGISTRO");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA_COM_REGISTRO && estadoAnterior != estadoAtual) {
        this->ligarLedAzul();
        Serial.println(">>> Estado: OCUPADA_COM_REGISTRO");
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO && estadoAnterior != estadoAtual) {
        this->ligarLedVermelha();
        Serial.println(">>> Estado: EM_MANUTENCAO");
    }
    else if (estadoAtual == EstadoEstacao::CONFIGURACAO) {
        if(estadoAtual != estadoAnterior) {
            Serial.println(">>> Estado: CONFIGURACAO");
        }
        this->ligarLedVerde();
        delay(50);
        this->ligarLedAmarela();
        delay(50);
        this->ligarLedAzul();
    }
}


///=== Métodos de processar eventos
float DispositivoController::lerDistancia() {
    this->executarPulso();
    long duracaoPulso = pulseIn(ECHO_PIN, HIGH);
    return (duracaoPulso * VELOCIDADE_DO_SOM) / 2;
}

void DispositivoController::executarPulso() {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}

//=== Métodos de renderizar
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

///=== Métodos utilitários
Aluno DispositivoController::obterAlunoPorTag(String tag) {
    for (int i = 0; i < 3; i++) {
        if(listaAlunos[i].tag.indexOf(tag) == 0) {
            return listaAlunos[i];
        }
    }
    return Aluno(0, "", "", "");
}