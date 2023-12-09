#ifndef __DISPLAY_H__
#define __DISPLAY_H__
#include <SPI.h>
#include <LCDWIKI_GUI.h>
#include <SSD1283A.h>

#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


/*
Pins layout SSD1283A ESP32
led   19
sck   18
sda   23
a0    22
rst   21
cs    5
vcc   3v3-5v
*/
class Display {
    private:
        SSD1283A_GUI display{/*CS=5*/ 5, /*DC=*/ 22, /*RST=*/ 21, /*LED=*/ 19}; 
    public:
        //=== Implementa padrão de projeto Singleton nessa classe
        Display() = default;
        Display(const Display&) = delete;
        Display(Display&&) = delete;
        Display& operator=(const Display&) = delete;
        Display& operator=(Display&&) = delete;
        ~Display() = default;

        void inicializar(void);
        void desenhar(void);
        void desenharStatusInicializando(void);
        void desenharStatusDisponivel(String nomeEstacao);
        void desenharStatusOcupada(String nomeEstacao, String matricula, String nomeAluno);
        void desenharStatusManutencao(String nomeEstacao);
        void atualizaTempo(int hour, int minute);
        void desenharCabecalho(void);
};
#endif