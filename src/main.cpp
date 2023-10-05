#include <Arduino.h>
#include <MFRC522.h>
#include "DispositivoController.h";


DispositivoController dispositivoController;
void setup() {
    dispositivoController.inicializar();    
    Serial.println("Projeto de monitoramento iniciado!");
}

void loop() {
    dispositivoController.processarEventos();
    dispositivoController.atualizar();
    dispositivoController.renderizar();
    delay(800);
}



