#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <HTTPClient.h>
#include "SPIFFS.h"
#include "DispositivoController.h"

//=== Métodos do Game Loop
void DispositivoController::inicializar() {
    Serial.begin(115200);
    SPI.begin();
    // SPIFFS.format();
    abrirSistemaDeArquivos();
    carregarEstacoesDeTrabalho();
    rfid.inicializar();
    display.inicializar();
    display.desenhar();
    botao.inicializar();
    conexao.inicializar();
    digitalWrite(2, LOW);
    conexao.conectar();
    if(conexao.estaConectado()) {
        HTTPClient http;
        String url = "https://io.adafruit.com/api/v2/time/ISO-8601";
        http.begin(url);

        // Adicionar cabeçalho de autenticação
        http.addHeader("X-AIO-Key", "aio_mpxW45Bxu9aR8cvCbvGIroCfx68q");

        // Enviar a requisição
        int httpCode = http.GET();

        // Verificar se a requisição foi bem-sucedida
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println("Resposta da API:");
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] Falha na requisição: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    produtor.inicializar();
    tempoTentativaAnterior = millis();
}

void DispositivoController::processarEventos() {
    produtor.manterConexao();
    botao.resetarBotao();
    botao.lerBotao();
    digitalWrite(12, LOW);
    digitalWrite(5, HIGH);
    rfid.ler();
    verificaConexaoEPublicaMensagens();
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
                Produtor::MensagemAlteracaoEstado msg(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
                bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
                if(publicado) {
                    Serial.println("Mensagem de alteração de estado publicada!");
                }
                Produtor::MensagemAcesso msgAcesso(estacaoDeTrabalho.id, alunoAtual);
                bool publicadoEntrada = produtor.publicarMensagemEntrada(msgAcesso);
                if(publicadoEntrada) {
                    Serial.println("Mensagem de entrada publicada!");
                }
                else {
                    adicionarMensagemAcessoEmArquivo(msgAcesso, "ENTRADA");
                }
            }
        }
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA) {
        if(!rfid.obterIdentificador().isEmpty() && rfid.obterIdentificador().indexOf(alunoAtual.tag) == 0) {
            // Só desloga o usuário caso a tag aproximada seja igual à tag do ocupante  
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
            if(publicado) {
                Serial.println("Mensagem de alteração de estado publicada!");
            }
            Produtor::MensagemAcesso msgAcesso(estacaoDeTrabalho.id, obterAlunoPorTag(alunoAtual.tag));
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada!");
            }
            else {
                adicionarMensagemAcessoEmArquivo(msgAcesso, "SAIDA");
            }
            alunoAtual = Aluno();
        }
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO) {
        if(!rfid.obterIdentificador().isEmpty() && rfid.obterIdentificador().indexOf(tagAdmin) == 0) {
            estadoAtual = EstadoEstacao::DISPONIVEL;
            Produtor::MensagemAlteracaoEstado msg(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
            bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
            if(publicado) {
                Serial.println("Mensagem de alteração de estado publicada!");
            }
        }
    }
    else if(estadoAtual == EstadoEstacao::CONFIGURACAO) {
        if(!rfid.obterIdentificador().isEmpty()) {
            EstacaoDeTrabalho edt = obterEstacaoDeTrabalhoPorTag(rfid.obterIdentificador());
            if(edt.id != 0) {
                estacaoDeTrabalho = edt;
                estadoAtual = ultimoEstadoPreConfiguracao == EstadoEstacao::DESLIGADO ? EstadoEstacao::DISPONIVEL : ultimoEstadoPreConfiguracao;
                if(estadoAtual == EstadoEstacao::DISPONIVEL) {
                    Produtor::MensagemAlteracaoEstado msg(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
                    bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
                    if(publicado) {
                        Serial.println("Mensagem de alteração de estado publicada!");
                    }
                }
            }

        }
    }
    
    if(botao.obterModo() == Botao::ModoBotao::SOLICITAR_MANUTENCAO) {
        estadoAtual = EstadoEstacao::EM_MANUTENCAO;
        Produtor::MensagemAlteracaoEstado msg(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
        bool publicado = produtor.publicarMensagemAlteracaoEstado(msg);
        if(publicado) {
            Serial.println("Mensagem de alteração de estado publicada com sucesso!");
        }
        if(!alunoAtual.tag.isEmpty()) {
            Produtor::MensagemAcesso msgAcesso(estacaoDeTrabalho.id, alunoAtual);
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada com sucesso!");
            }
            else {
                adicionarMensagemAcessoEmArquivo(msgAcesso, "SAIDA");
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
        display.desenharStatusDisponivel(estacaoDeTrabalho.nome);
        Serial.println(">>> Estado: DISPONIVEL");
    }
    else if(estadoAtual == EstadoEstacao::OCUPADA && estadoAnterior != estadoAtual) {
        display.desenharStatusOcupada(estacaoDeTrabalho.nome,alunoAtual.matricula, alunoAtual.nome);
        Serial.println(">>> Estado: OCUPADA");
    }
    else if(estadoAtual == EstadoEstacao::EM_MANUTENCAO && estadoAnterior != estadoAtual) {
        display.desenharStatusManutencao(estacaoDeTrabalho.nome);
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

void DispositivoController::escreverEmArquivo(String payload, String caminho, String modo) {
  File rFile = SPIFFS.open(caminho, modo.c_str());
  if (!rFile) {
    Serial.println("Erro ao abrir arquivo!");
  }
  else {
    Serial.print("Gravou: ");
    Serial.println(payload);
    rFile.println(payload);
  }
  rFile.close();
}

String DispositivoController::lerArquivo(String caminho) {
    File rFile = SPIFFS.open(caminho, "r");
    String arquivo = "";
    if (!rFile) {
        Serial.println("Erro ao abrir arquivo!");
        return "";
    }
    else {
        Serial.print("----------Lendo arquivo ");
        Serial.print(caminho);
        Serial.println("  ---------");
        while (rFile.position() < rFile.size())
        {
            arquivo = rFile.readStringUntil('\n');
            arquivo.trim();
            Serial.print(">>> Li:");
            Serial.println(arquivo);
        }
        rFile.close();
        return arquivo;
    }
}

void DispositivoController::abrirSistemaDeArquivos() {
  if (!SPIFFS.begin()) {
    Serial.println("\nErro ao abrir o sistema de arquivos");
  }
  else {
    Serial.println("\nSistema de arquivos aberto com sucesso!");
  }
}

void DispositivoController::verificaConexaoEPublicaMensagens() {
    long tempoTentativaAtual = millis();
    if(tempoTentativaAtual - tempoTentativaAnterior >= tempoPoliticaDeReconexao) {
        // Adota política de reconexão em caso de instabilidade
        if(!conexao.estaConectado()) {
            Serial.println("Não conectado à rede. Conectando...");
            conexao.conectar();
        }
        if(conexao.estaConectado() && !produtor.estaConectado()) {
            Serial.println("Produtor não conectado ao broker.");
            produtor.conectar();
            publicarMensagensAtrasadas();
        }
        tempoTentativaAnterior = tempoTentativaAtual;
    }
}

void arquivoParaLista(String arquivo, char delimitador, String lista[], int tamanhoMaximo) {
    int indiceInicial = 0;
    int indiceFinal = arquivo.indexOf(delimitador, indiceInicial);
    int indiceLista = 0;

    while (indiceFinal != -1 && indiceLista < tamanhoMaximo) {
        // Extrai a substring entre os delimitadores
        String itemAtual = arquivo.substring(indiceInicial, indiceFinal);

        // Adiciona o item à lista
        lista[indiceLista] = itemAtual;

        // Atualiza os índices para a próxima iteração
        indiceInicial = indiceFinal + 1;
        indiceFinal = arquivo.indexOf(delimitador, indiceInicial);

        // Incrementa o índice da lista
        indiceLista++;
    }
}

void DispositivoController::adicionarMensagemAcessoEmArquivo(Produtor::MensagemAcesso msg, String tipo) {
    String registrosAlunos = lerArquivo("/registros.txt");
    String novoRegistro;
    novoRegistro.concat(String(msg.idEstacao));
    novoRegistro.concat(",");
    novoRegistro.concat(String(msg.aluno.id));
    novoRegistro.concat(",");
    novoRegistro.concat(msg.aluno.nome);
    novoRegistro.concat(",");
    novoRegistro.concat(msg.aluno.matricula);
    novoRegistro.concat(",");
    novoRegistro.concat(tipo);
    novoRegistro.concat("%");
    registrosAlunos.concat(novoRegistro);
    escreverEmArquivo(registrosAlunos, "/registros.txt", "w");
}

/*
    Tenta publicar as mensagens presentes no arquivo registros.txt
    e também publica o último estado da aplicação para sincronizar
    caso não tenha sido possível enviar antes.
    Caso alguma mensagem não for publicada, será mantida no arquivo
    registros.txt e tentará ser enviada novamente na próxima chamada.
*/
void DispositivoController::publicarMensagensAtrasadas() {
    Produtor::MensagemAlteracaoEstado msgEstado(estacaoDeTrabalho.id, estacaoDeTrabalho.nome, estadoEstacaoEnumStr[static_cast<int>(estadoAtual)]);
    if(estadoAtual != EstadoEstacao::DESLIGADO || estadoAtual != EstadoEstacao::CONFIGURACAO) {
        bool publicado = produtor.publicarMensagemAlteracaoEstado(msgEstado);
        if(publicado) {
            Serial.println("Mensagem de alteração de estado publicada com sucesso!");
        }
    }
    delay(200);
    String registrosAlunos = lerArquivo("/registros.txt");
    int tamanhoRegistrosParseados = 10;
    String registrosParseados[tamanhoRegistrosParseados];
    arquivoParaLista(registrosAlunos, '%', registrosParseados,tamanhoRegistrosParseados);
    int mensagensNaoPublicadas[tamanhoRegistrosParseados];
    int indiceMensagensNaoPublicadas = 0;
    for(int i = 0; i < tamanhoRegistrosParseados; i++) {
        // idEstacao, idAluno, nomeAluno, matriculaAluno, tipo
        int tamanhoRegistroAtual = 5;
        String registroAtual[tamanhoRegistroAtual]; 
        arquivoParaLista(registrosParseados[i],',', registroAtual,tamanhoRegistroAtual);
        long idEstacao = atoi(registroAtual[0].c_str());
        long idAluno = atoi(registroAtual[1].c_str());
        String nomeAluno = registroAtual[2];
        String matriculaAluno = registroAtual[3];
        String tipo = registroAtual[4];
        Aluno aluno(idAluno, nomeAluno, matriculaAluno, "");
        Produtor::MensagemAcesso msgAcesso(idEstacao, aluno);
        if(tipo.indexOf("ENTRADA") == 0) {
            bool publicadoEntrada = produtor.publicarMensagemEntrada(msgAcesso);
            if(publicadoEntrada) {
                Serial.println("Mensagem de entrada publicada com sucesso!");
            }
            else {
                mensagensNaoPublicadas[indiceMensagensNaoPublicadas] = i;
                indiceMensagensNaoPublicadas++;
            }
        }else {
            bool publicadoSaida = produtor.publicarMensagemSaida(msgAcesso);
            if(publicadoSaida) {
                Serial.println("Mensagem de saída publicada com sucesso!");
            }
            else {
                mensagensNaoPublicadas[indiceMensagensNaoPublicadas] = i;
                indiceMensagensNaoPublicadas++;
            }
            delay(200);
        } 
        // Caso alguma mensagem não tenha sido publicada, tentará publicar novamente
        // na próxima vez que entrar na política de sincronização
        if(indiceMensagensNaoPublicadas > 0) {
            String novoArquivo;
            for(int j = 0; j < indiceMensagensNaoPublicadas; j++) {
                novoArquivo.concat(registrosParseados[mensagensNaoPublicadas[j]]);
                novoArquivo.concat("%");
            }
            escreverEmArquivo(novoArquivo, "/registros.txt", "w");
        }
    }
}

void DispositivoController::carregarEstacoesDeTrabalho() {
    const EstacaoDeTrabalho listaEstacoesDeTrabalho[3] = {
        EstacaoDeTrabalho(1, "PC201-01", " a9 4f 89 1f"),
        EstacaoDeTrabalho(2, "PC201-02", " 93 bc 46 13"),
        EstacaoDeTrabalho(3, "PC201-03", " a6 ed 96 57")
    };
    String arquivoEstacoesDeTrabalho;
    for(int i = 0; i < 3; i++) {
        String estacaoAtual;
        estacaoAtual.concat(String(listaEstacoesDeTrabalho[i].id));
        estacaoAtual.concat(",");
        estacaoAtual.concat(String(listaEstacoesDeTrabalho[i].nome));
        estacaoAtual.concat(",");
        estacaoAtual.concat(String(listaEstacoesDeTrabalho[i].tag));
        estacaoAtual.concat(",");
        estacaoAtual.concat("%");
        arquivoEstacoesDeTrabalho.concat(estacaoAtual);
    }
    escreverEmArquivo(arquivoEstacoesDeTrabalho, "/estacoesDeTrabalho.txt", "w");
}

EstacaoDeTrabalho DispositivoController::obterEstacaoDeTrabalhoPorTag(String tag) {
    EstacaoDeTrabalho estacao;
    String arquivo = lerArquivo("/estacoesDeTrabalho.txt");
    int tamanhoEstacoesParseadas = 3;
    String estacoesParseadas[tamanhoEstacoesParseadas]; 
    arquivoParaLista(arquivo, '%', estacoesParseadas, tamanhoEstacoesParseadas);
    for(int i = 0; i < tamanhoEstacoesParseadas; i++) {
        int tamanhoEstacaoAtual = 3;
        String estacaoAtual[tamanhoEstacaoAtual];
        arquivoParaLista(estacoesParseadas[i],',', estacaoAtual, tamanhoEstacaoAtual);
        String tagEstacaoAtual = estacaoAtual[2];
        if(tagEstacaoAtual.indexOf(tag) == 0) {
            long id = atoi(estacaoAtual[0].c_str());
            String nome = estacaoAtual[1];
            estacao = EstacaoDeTrabalho(id, nome, tag);
        }
    }
    return estacao;
}