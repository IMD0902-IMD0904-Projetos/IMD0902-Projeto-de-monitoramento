#ifndef __BOTAO_H__
#define __BOTAO_H__
class Botao {
    public:
        enum class ModoBotao: short {
            DESLIGADO=0,
            SAIDA,
            SOLICITAR_MANUTENCAO,
            CONFIGURACAO
        };
    private:
        //=== Estados possíveis do botão
        enum class EstadoBotao: short {
            NAO_PRESSIONADO=0,
            PRESSIONADO
        };
        const int BOTAO_PIN = 14;
        unsigned long inicioTempoPressao = 0;
        EstadoBotao estadoAtual;
        EstadoBotao estadoAnterior;
        ModoBotao modo = ModoBotao::DESLIGADO;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        Botao() = default;
        Botao(const Botao&) = delete;
        Botao(Botao&&) = delete;
        Botao& operator=(const Botao&) = delete;
        Botao& operator=(Botao&&) = delete;
        ~Botao() = default;

        //=== Inicializa as configurações necessárias para o programa rodar.
        void inicializar(void);
        //=== Faz leitura do botão para saber se está pressionado.
        void lerBotao(void);
        //=== Reinicia o modo do botão para desligado.
        void resetarBotao(void);
        //=== Obter modo do botão.
        ModoBotao obterModo(void) const;

    private:
        //=== Define o modo do botão com base na duração.
        void definirModo(const unsigned long& duracao);
};
#endif