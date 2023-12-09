#ifndef __ESTACAO_DE_TRABALHO_H__
#define __ESTACAO_DE_TRABALHO_H__
#include <Arduino.h>
class EstacaoDeTrabalho {
    public:
        long id;
        String nome;
        String tag;
    public:
        EstacaoDeTrabalho() = default;
        ~EstacaoDeTrabalho() = default;
        EstacaoDeTrabalho(long _id, String _nome, String _tag) {
            id = _id;
            nome = _nome;
            tag = _tag;
        }
};
#endif