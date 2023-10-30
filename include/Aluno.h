#ifndef __ALUNO_H__
#define __ALUNO_H__
#include <Arduino.h>
// TODO Remover quando a informação estiver salva em um BD.
class Aluno {
    public:
        long id;
        String nome;
        String matricula;
        String tag;
    public:
        Aluno() = default;
        ~Aluno() = default;
        Aluno(long _id, String _nome, String _matricula, String _tag) {
            id = _id;
            nome = _nome;
            matricula = _matricula;
            tag = _tag;
        }
};
#endif