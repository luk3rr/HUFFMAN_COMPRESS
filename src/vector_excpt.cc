/*
* Filename: vector_excpt.cc
* Created on: June  5, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include "vector_excpt.h"

const char* vecexcpt::InvalidIndex::what() const throw() {
    return "ERRO: Índice inválido";
}

const char* vecexcpt::VectorIsEmpty::what() const throw() {
    return "ERRO: O vector está vazio";
}
