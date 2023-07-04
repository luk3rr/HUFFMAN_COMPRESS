/*
* Filename: parser.h
* Created on: July  4, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#ifndef PARSER_H_
#define PARSER_H_

#include <fstream>
#include <string>

#include "huffman_compress_excpt.h"

namespace huff {
    class Parser {
      public:
            /**
             * @brief Diz se um arquivo está codificado como UTF-8
             * @param file Nome do arquivo que será verificado
             **/
            static bool IsUTF8(std::ifstream &file);

            /**
             * @brief Diz se um arquivo contém a assinatura do programa
             *        Somente arquivos que contém essa assinatura podem ser descompactados
             * @param file Arquivo que será lido
             **/
            static bool CheckSignature(std::ifstream &file);

            /**
             * @brief Determina se o arquivo binário é compátivel com este programa, isto é, foi gerado por ele e pode ser
             *        descompactado
             * @param file Nome do arquivo que será verificado
             **/
            static bool CheckDecodeCompatibility(std::string file);

            /**
             * @brief Determina se o arquivo de texto é compátivel com este programa, isto é, pode ser compactado por ele
             * @param file Nome do arquivo que será verificado
             **/
            static bool CheckEncodeCompatibility(std::string file);
    };
}

#endif // PARSER_H_
