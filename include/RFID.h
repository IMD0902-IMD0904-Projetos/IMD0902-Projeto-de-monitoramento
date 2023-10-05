#ifndef __RFID_H__
#define __RFID_H__
#include "MFRC522.h"

class RFID {
    private:
        const int SS_PIN = 5;
        const int RST_PIN = 15;
        MFRC522 rfid;
        String identificador;
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        RFID() = default;
        RFID(const RFID&) = delete;
        RFID(RFID&&) = delete;
        RFID& operator=(const RFID&) = delete;
        RFID& operator=(RFID&&) = delete;
        ~RFID() = default;

        //=== Inicializa as configurações para o módulo funcionar.
        void inicializar(void);
        //=== Método que tenta ler o RFID e coloca valor na variável identificador.
        void ler(void);
        //=== Método que limpa identificador para próxima leitura.
        void limparIdentificador(void);
        //=== Obtém o identificador do RFID
        String obterIdentificador(void) const;
};
#endif