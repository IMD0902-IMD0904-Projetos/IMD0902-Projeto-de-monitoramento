#ifndef __PRODUTOR_H__
#define __PRODUTOR_H__
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
class Produtor {
    private:
        const char* IO_USERNAME = "fawnbr";
        const char* IO_KEY = "aio_xIas91DlKduxcjkgHPHc288W1rDM";
        const char* TOPICO_ACESSO = "fawnbr/feeds/projetomonitoramento.acesso";
        const char* TOPICO_ALTERACAO_ESTADO = "fawnbr/feeds/projetomonitoramento.alteracaoestado";
        const char* MQTT_BROKER = "io.adafruit.com";
        const int MQTT_PORT = 1883;
        const int MQTT_TIMEOUT = 10000;
        WiFiClient* esp32WiFiClient = nullptr;
        PubSubClient* mqttClient = nullptr;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        Produtor() = default;
        Produtor(const Produtor&) = delete;
        Produtor(Produtor&&) = delete;
        Produtor& operator=(const Produtor&) = delete;
        Produtor& operator=(Produtor&&) = delete;
        ~Produtor() = default;

        // TODO Remover quando a informação estiver salva em um BD.
        struct Aluno {
            long id;
            const char* nome;
            const char* matricula;
            const char* tag;

            // Construtor
            Aluno(long _id, const char* _nome, const char* _matricula, const char* _tag)
                : id(_id), nome(_nome), matricula(_matricula), tag(_tag) {}
            ~Aluno() = default;
        };

        struct MensagemAlteracaoEstado {
            long id;
            const char* nome;
            const char* estado;
            // Construtor
            MensagemAlteracaoEstado(const long id, const char* nome, const char* estado) : id(id), nome(nome), estado(estado) {}
            ~MensagemAlteracaoEstado() = default;
        };

        struct MensagemAcesso {
            long idEstacao;
            Aluno aluno;
            // Construtor
            MensagemAcesso(const long idEstacao, Aluno* aluno) : 
                idEstacao(idEstacao), aluno(*aluno) {}
            ~MensagemAcesso() = default;
        };

        //=== Inicializa as configurações e variáveis necessárias para o programa rodar.
        void inicializar(void);
        void conectar(void);
        bool estaConectado(void);
        bool publicarMensagemAlteracaoEstado(const MensagemAlteracaoEstado* mensagem);
        bool publicarMensagemEntrada(const MensagemAcesso* mensagem);
        bool publicarMensagemSaida(const MensagemAcesso* mensagem);

        //=== Utils
        const char* obterMomentoAtual(void);
        String montarJSONAcesso(const MensagemAcesso* mensagem, const char* tipo);
        String montarJSONAlteracaoEstado(const MensagemAlteracaoEstado* mensagem);
};
#endif