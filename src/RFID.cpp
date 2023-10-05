#include "RFID.h";

void RFID::inicializar() {
    rfid = MFRC522{SS_PIN, RST_PIN};
    rfid.PCD_Init();
}

void RFID::ler() {
    if (rfid.PICC_IsNewCardPresent()) { 
        if (rfid.PICC_ReadCardSerial()) {
            MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
            Serial.print("RFID/NFC Tag Type: ");
            Serial.println(rfid.PICC_GetTypeName(piccType));
            Serial.print("UID:");
            for (int i = 0; i < rfid.uid.size; i++) {
                Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
                Serial.print(rfid.uid.uidByte[i], HEX);
            }
            Serial.println();

            rfid.PICC_HaltA(); // halt PICC
            rfid.PCD_StopCrypto1(); // stop encryption on PCD
        }
    }
}