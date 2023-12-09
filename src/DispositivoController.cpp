#include <Arduino.h>
#include <SPI.h>
#include <cstring>
#include "DispositivoController.h"

//=== Métodos do Game Loop
void DispositivoController::inicializar() {
    Serial.begin(115200);
    SPI.begin();
    rfid.inicializar();
    display.inicializar();
    display.desenhar();
    botao.inicializar();
    conexao.inicializar();
    digitalWrite(2, LOW);
    produtor.inicializar();
}

void DispositivoController::processarEventos() {
    botao.resetarBotao();
    botao.lerBotao();
    digitalWrite(12, LOW);
    digitalWrite(5, HIGH);
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
        estadoAtual = EstadoEstacao::CONFIGURACAO;
    }
    else if(estadoAtual == EstadoEstacao::DISPONIVEL) {
        if(!rfid.obterIdentificador().isEmpty()) {
            alunoAtual = obterAlunoPorTag(rfid.obterIdentificador());
            if(alunoAtual.id != 0) {
                estadoAtual = EstadoEstacao::OCUPADA;
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

    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        if(!rfid.obterIdentificador().isEmpty() && rfid.obterIdentificador().indexOf(alunoAtual.tag) == 0) {
            // Só desloga o usuário caso a tag aproximada seja igual à tag do ocupante  
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
            if(publicado) {
                Serial.println("Mensagem de alteração de estado publicada!");
            }
            Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, obterAlunoPorTag(alunoAtual.tag));
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada!");
            }
            alunoAtual = Aluno();
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
            estadoAtual = ultimoEstadoPreConfiguracao == EstadoEstacao::DESLIGADO ? EstadoEstacao::DISPONIVEL : ultimoEstadoPreConfiguracao;
            if(estadoAtual == EstadoEstacao::DISPONIVEL) {
                Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
                bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
                if(publicado) {
                    Serial.println("Mensagem de alteração de estado publicada!");
                }
            }
        }
    }
    
    if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
        estadoAtual = EstadoEstacao::EM_MANUTENCAO;
        Produtor::MensagemAlteracaoEstado msg(ID_DISPOSITIVO, NOME_DISPOSITIVO, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
        bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
        if(publicado) {
            Serial.println("Mensagem de alteração de estado publicada com sucesso!");
        }
        if(!alunoAtual.tag.isEmpty()) {
            Produtor::MensagemAcesso msgAcesso(ID_DISPOSITIVO, alunoAtual);
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada com sucesso!");
            }
            alunoAtual = Aluno();
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

int hour = 15;
int minute = 54;
long lastTime = millis();
void DispositivoController::renderizar() {
    digitalWrite(12, HIGH);
    digitalWrite(5, LOW);
    long currentTime = millis();
    if(currentTime - lastTime >= 3000) {
        lastTime = currentTime;
        minute++;
        if(minute % 60 == 0) {
            hour++;
            minute = 0;
            if(hour % 24 == 0) {
                hour = 0;
            }
        }
        display.atualizaTempo(hour, minute);
    }
    if(estadoAtual == EstadoEstacao::DISPONIVEL && estadoAnterior != estadoAtual) {
        display.desenharStatusDisponivel(NOME_DISPOSITIVO);
        Serial.println(">>> Estado: DISPONIVEL");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA && estadoAnterior != estadoAtual) {
        display.desenharStatusOcupada(NOME_DISPOSITIVO,alunoAtual.matricula, alunoAtual.nome);
        Serial.println(">>> Estado: OCUPADA");
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO && estadoAnterior != estadoAtual) {
        display.desenharStatusManutencao(NOME_DISPOSITIVO);
        Serial.println(">>> Estado: EM_MANUTENCAO");
    }
    else if (estadoAtual == EstadoEstacao::CONFIGURACAO) {
        if(estadoAtual != estadoAnterior) {
            display.desenharStatusInicializando();
            Serial.println(">>> Estado: CONFIGURACAO");
        }
    }
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