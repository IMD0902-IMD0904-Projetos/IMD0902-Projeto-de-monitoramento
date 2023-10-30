#ifndef __DISPOSITIVO_CONTROLLER_H__
#define __DISPOSITIVO_CONTROLLER_H__
#include "RFID.h"
#include "Botao.h"
#include "Conexao.h"
#include "Produtor.h"
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
            OCUPADA_SEM_REGISTRO,
            OCUPADA_COM_REGISTRO,
            EM_MANUTENCAO,
            DESLIGADO,
            CONFIGURACAO
        };
        const char* estadoEstacaoEnumStr[5] = {
            "DISPONIVEL", 
            "OCUPADA_SEM_REGISTRO",
            "OCUPADA_COM_REGISTRO", 
            "EM_MANUTENCAO",
            "DESLIGADO"
        };

        const Aluno listaAlunos[3] = {
            Aluno(1, "Lucas Vinícius Góis Nogueira", "20210076805", " a9 4f 89 1f"),
            Aluno(2, "Lourrayni Feliph Querino de Araujo Dantas", "20230032693", "ler"),
            Aluno(3, "Pedro Lucas Góis Costa", "20210056750", "ler")
        };

        EstadoEstacao estadoAtual = EstadoEstacao::DESLIGADO;
        EstadoEstacao estadoAnterior = EstadoEstacao::DESLIGADO;
        EstadoEstacao ultimoEstadoPreConfiguracao = EstadoEstacao::DESLIGADO;
        String tagAdmin = " 93 bc 46 13";
        String tagLogada;
        float distancia;
        RFID rfid;
        Botao botao;
        Conexao conexao;
        Produtor produtor;
        // Em cm/uS
        const float VELOCIDADE_DO_SOM = 0.034;
        const float DISTANCIA_TRIGGER = 15.0;
        const int TRIGGER_PIN = 13;
        const int ECHO_PIN = 12;
        const int LED_VERDE_PIN = 27;
        const int LED_AMARELA_PIN = 26;
        const int LED_AZUL_PIN = 25;
        const int LED_VERMELHA_PIN = 33;
        // Informações da estação de trabalho
        const long ID_DISPOSITIVO = 1L;
        const char* NOME_DISPOSITIVO = "PC202-01";
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
        //=== Renderiza algo no terminal / Manipula leds 
        void renderizar(void) const;

    private:
        //=== Lê distância do sensor ultrassônico. Valor em cm/uS.
        float lerDistancia(void);
        //=== Executa pulso ultrassônico para ser lido pelo método lerDistancia()
        void executarPulso(void);
        Aluno obterAlunoPorTag(String tag);
        void ligarLedAzul(void) const;  
        void ligarLedAmarela(void) const;
        void ligarLedVerde(void) const;
        void ligarLedVermelha(void) const;  

};
#endif