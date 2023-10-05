#ifndef __RFID_H__
#define __RFID_H__
#include "MFRC522.h";

class RFID {
    private:
        const int SS_PIN = 5;
        const int RST_PIN = 15;
        MFRC522 rfid;
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
        // FIXME Método temporário que só exibe o id do RFID na saída padrão
        void ler(void);
};
#endif