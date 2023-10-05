#ifndef __DISPOSITIVO_CONTROLLER_H__
#define __DISPOSITIVO_CONTROLLER_H__
#include "RFID.h";
/*!
* Essa classe representa o controlador do dispositivo que é responsável por
* rastrear, processar e manipular os dados recebidos pelo sensor de distância
* ultrassônico, botão e módulo de leitor RFID.
*
* Essa classe implementa um padrão de projeto muito comum na área de jogos
* chamado de Game Loop. Ver mais em: https://java-design-patterns.com/patterns/game-loop/
*/
class DispositivoController {
    private:
        enum class EstadoEstacao : short {
            DISPONIVEL = 0,
            OCUPADA_SEM_REGISTRO,
            OCUPADA,
            EM_MANUTENCAO
        };

        EstadoEstacao estadoAtual;
        EstadoEstacao estadoAnterior;
        float distancia;
        RFID rfid;
        // Em cm/uS
        const float VELOCIDADE_DO_SOM = 0.034;
        const float DISTANCIA_TRIGGER = 40.0;
        const int TRIGGER_PIN = 13;
        const int ECHO_PIN = 12;
        const int BOTAO_PIN = 14;
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
        //=== Esses métodos que começam em lerXXX() são usados dentro de processarEventos()
        float lerDistancia();
        int lerBotao();

        //=== Executa pulso ultrassônico para ser lido pelo método lerDistancia()
        void executarPulso();

};
#endif