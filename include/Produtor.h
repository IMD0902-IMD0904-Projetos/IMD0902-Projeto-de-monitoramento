#ifndef __PRODUTOR_H__
#define __PRODUTOR_H__
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "Aluno.h"

class Produtor {
    private:
        // Informações do broker Adafruit
        const char* IO_USERNAME = "fawnbr";
        const char* IO_KEY = "ai_key";
        const char* TOPICO_ACESSO = "fawnbr/feeds/projetomonitoramento.acesso";
        const char* TOPICO_ALTERACAO_ESTADO = "fawnbr/feeds/projetomonitoramento.alteracaoestado";
        const char* MQTT_BROKER = "io.adafruit.com";
        // Informações do MQTT
        String clientId = "ESP32ClientProjetoMonitoramento_";
        const int MQTT_PORT = 1883;
        const int MQTT_TIMEOUT = 10000;
        WiFiClient esp32WiFiClient;
        PubSubClient mqttClient;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        Produtor() = default;
        Produtor(const Produtor&) = delete;
        Produtor(Produtor&&) = delete;
        Produtor& operator=(const Produtor&) = delete;
        Produtor& operator=(Produtor&&) = delete;
        ~Produtor() = default;

        struct MensagemAlteracaoEstado {
            int id;
            String nome;
            String estado;
            // Construtor
            MensagemAlteracaoEstado(int _id, String _nome, String _estado)
            {
                id = _id;
                nome = _nome;
                estado = _estado;
            }
            ~MensagemAlteracaoEstado() = default;
        };

        struct MensagemAcesso {
            long idEstacao;
            Aluno aluno;
            // Construtor
            MensagemAcesso(const long _idEstacao, Aluno _aluno) {
                idEstacao = _idEstacao;
                aluno = _aluno;
            }
            ~MensagemAcesso() = default;
        };

        //=== Inicializa as configurações e variáveis necessárias para o programa rodar.
        void inicializar(void);
        void conectar(void);
        bool manterConexao(void);
        bool estaConectado(void);
        bool publicarMensagemAlteracaoEstado(const MensagemAlteracaoEstado mensagem);
        bool publicarMensagemEntrada(const MensagemAcesso mensagem);
        bool publicarMensagemSaida(const MensagemAcesso mensagem);

        // //=== Utils
        String montarJSONAcesso(const MensagemAcesso mensagem, String tipo);
        String montarJSONAlteracaoEstado(const MensagemAlteracaoEstado mensagem);
};
#endif