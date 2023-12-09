#include "RFID.h"

void RFID::inicializar() {
    rfid = MFRC522{SS_PIN, RST_PIN};
    rfid.PCD_Init();
}

void RFID::ler() {
    if (rfid.PICC_IsNewCardPresent()) {
        if (rfid.PICC_ReadCardSerial()) {
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
            Serial.print("| [RFID/NFC Tag Type]: ");
            Serial.print(String(rfid.PICC_GetTypeName(piccType)) + " | ID:");
            for (int i = 0; i < rfid.uid.size; i++) {
                identificador.concat(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                identificador.concat(String(rfid.uid.uidByte[i], HEX));
            }
            Serial.println(identificador);
            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
        }
    }
}

String RFID::obterIdentificador() const {
    return identificador;
}

void RFID::limparIdentificador() {
    identificador = "";
}