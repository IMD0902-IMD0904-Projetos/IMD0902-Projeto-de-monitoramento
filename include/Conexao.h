#ifndef __CONEXAO_H__
#define __CONEXAO_H__
class Conexao {
    private:
        const char* WIFI_SSID = "ssid";
        const char* WIFI_PASSWORD = "password";
        const int WIFI_TIMEOUT = 10000;
        const int LED_CONEXAO_PIN = 2;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        Conexao() = default;
        Conexao(const Conexao&) = delete;
        Conexao(Conexao&&) = delete;
        Conexao& operator=(const Conexao&) = delete;
        Conexao& operator=(Conexao&&) = delete;
        ~Conexao() = default;
        
        //=== Inicializa configuração WiFi no modo STA.
        void inicializar(void);
        //=== Tenta se conectar na rede com uma política de retry de 20s;
        void conectar(void);
        //=== Verifica se o dispositivo está conectado à internet.
        bool estaConectado(void);
    
};
#endif