#include "Display.hpp"
#include <LCDWIKI_SPI.h>

void Display::inicializar() {
    display.init();
    display.Fill_Screen(BLACK);
}

void Display::atualizaTempo(int hour, int minute) {
    String displayTime = "";
    String hourStr = "";
    hourStr.concat(hour < 10 ? "0"+String(hour) : String(hour));
    displayTime.concat(hourStr);
    displayTime.concat(String(":"));
    String minuteStr = "";
    minuteStr.concat(minute < 10 ? "0"+String(minute) : String(minute));
    displayTime.concat(minuteStr);
    
    String fullDateTime = "";
    fullDateTime.concat(displayTime);
    fullDateTime.concat(" - 8-Dez-2023");
    display.Set_Text_Mode(0);
    display.Set_Text_colour(WHITE);
    display.Set_Text_Size(1);
    display.Print_String(fullDateTime, 0, 10);
}

void Display::desenharCabecalho() {
    display.Set_Text_Mode(0);
    display.Fill_Screen(BLACK);
    display.Set_Text_colour(WHITE);
    display.Set_Text_Back_colour(BLACK);
    display.Set_Text_Size(1);
    display.Print_String("IMD0904-nPITI-2023.2", 0, 0);
}

void Display::desenharStatusInicializando() {
    desenharCabecalho();
    display.Set_Text_colour(WHITE);
    display.Set_Text_Size(2);
    display.Print_String("Aproxime a", 0, 25);
    display.Print_String("tag RFID", 0, 43);
    display.Print_String("da Estacao", 0, 61);
    display.Print_String("de Trabalho", 0, 79);
}

void Display::desenharStatusDisponivel(String nomeEstacao) {
    desenharCabecalho();
    display.Set_Text_colour(YELLOW);
    display.Set_Text_Size(1);
    display.Print_String("Estacao: "+nomeEstacao, 0, 20);
    display.Set_Text_colour(GREEN);
    display.Set_Text_Size(2);
    display.Print_String("DISPONIVEL", 0, 30);
    display.Set_Text_colour(WHITE);
    display.Print_String("Aproxime a", 0, 48);
    display.Print_String("tag RFID", 0, 66);
    display.Print_String("pra iniciar", 0, 84);
    display.Print_String("o registro", 0, 102);
}

void Display::desenharStatusOcupada(String nomeEstacao, String matricula, String nomeAluno) {
    desenharCabecalho();
    display.Set_Text_colour(YELLOW);
    display.Set_Text_Size(1);
    display.Print_String("Estacao: "+nomeEstacao, 0, 20);
    display.Set_Text_colour(CYAN);
    display.Set_Text_Size(2);
    display.Print_String("OCUPADA", 0, 30);
    display.Set_Text_colour(WHITE);
    display.Print_String("Matricula:", 0, 48);
    display.Print_String(matricula, 0, 66);
    display.Print_String("Nome:", 0, 84);
    display.Print_String(nomeAluno, 0, 102);
}

void Display::desenharStatusManutencao(String nomeEstacao) {
    desenharCabecalho();
    display.Set_Text_colour(YELLOW);
    display.Set_Text_Size(1);
    display.Print_String("Estacao: "+nomeEstacao, 0, 20);
    display.Set_Text_colour(RED);
    display.Set_Text_Size(2);
    display.Print_String("MANUTENCAO", 0, 30);
    display.Set_Text_colour(WHITE);
    display.Print_String("Aproxime a", 0, 48);
    display.Print_String("tag RFID", 0, 66);
    display.Print_String("admin para", 0, 84);
    display.Print_String("liberar", 0, 102);
}

void Display::desenhar() {
    desenharCabecalho();
    display.Set_Text_colour(WHITE);
    display.Set_Text_Size(2);
    display.Print_String("Iniciando", 0, 25);
    display.Print_String("o projeto", 0, 43);
}