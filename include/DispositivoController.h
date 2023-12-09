#ifndef __DISPOSITIVO_CONTROLLER_H__
#define __DISPOSITIVO_CONTROLLER_H__
#include "RFID.h"
#include "Botao.h"
#include "Conexao.h"
#include "Produtor.h"
#include "Display.hpp"
#include "EstacaoDeTrabalho.h"
/*
* Essa classe representa o controlador do dispositivo que é responsável por
* rastrear, processar e manipular os dados recebidos pelo sensor de distância
* ultrassônico, botão e módulo de leitor RFID.
*
* Essa classe implementa um padrão de projeto muito comum na área de jogos
* chamado de Game Loop. Ver mais em: https://java-design-patterns.com/patterns/game-loop/
*/
class DispositivoController {
    private:
        enum class EstadoEstacao : int {
            DISPONIVEL = 0,
            OCUPADA,
            EM_MANUTENCAO,
            DESLIGADO,
            CONFIGURACAO
        };
        const char* estadoEstacaoEnumStr[4] = {
            "DISPONIVEL", 
            "OCUPADA", 
            "EM_MANUTENCAO",
            "DESLIGADO"
        };

        const Aluno listaAlunos[3] = {
            Aluno(1, "Lucas Vinícius Góis Nogueira", "20210076805", " a9 4f 89 1f"),
            Aluno(2, "Lourrayni Feliph Querino de Araujo Dantas", "20230032693", " 93 bc 46 13"),
            Aluno(3, "Pedro Lucas Góis Costa", "20210056750", " a6 ed 96 57")
        };

        EstadoEstacao estadoAtual = EstadoEstacao::DESLIGADO;
        EstadoEstacao estadoAnterior = EstadoEstacao::DESLIGADO;
        EstadoEstacao ultimoEstadoPreConfiguracao = EstadoEstacao::DESLIGADO;
        String tagAdmin = " 93 bc 46 13";
        RFID rfid;
        Botao botao;
        Conexao conexao;
        Produtor produtor;
        Display display;
        // Informações da política de reconexão ao WiFi ou AdafruitIO
        long tempoPoliticaDeReconexao = 30000;
        long tempoTentativaAnterior;
        // Informações da estação de trabalho
        EstacaoDeTrabalho estacaoDeTrabalho;
        Aluno alunoAtual;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        DispositivoController() = default;
        DispositivoController(const DispositivoController&) = delete;
        DispositivoController(DispositivoController&&) = delete;
        DispositivoController& operator=(const DispositivoController&) = delete;
        DispositivoController& operator=(DispositivoController&&) = delete;
        ~DispositivoController() = default;

        //=== Inicializa as configurações e variáveis necessárias para o programa rodar.
        void inicializar(void);
        //=== Processa eventos de entrada do usuário a partir dos módulos RFID, botão e sensor de distância.
        void processarEventos(void);
        //=== Atualiza a aplicação com base no estado atual
        void atualizar(void);
        //=== Renderiza algo no terminal / Manipula display 
        void renderizar(void);

    private:
        Aluno obterAlunoPorTag(String tag);
        // === Arquivo utils (SPIFFS)
        void escreverEmArquivo(String payload, String caminho, String modo);
        String lerArquivo(String caminho);
        void abrirSistemaDeArquivos(void);
        // === Política de reconexão e sincronização utils
        void verificaConexaoEPublicaMensagens(void);
        void publicarMensagensAtrasadas(void);
        void adicionarMensagemAcessoEmArquivo(Produtor::MensagemAcesso msg, String tipo);
        // === Estações de trabalho utils
        void carregarEstacoesDeTrabalho(void);
        EstacaoDeTrabalho obterEstacaoDeTrabalhoPorTag(String tag);
};
#endif