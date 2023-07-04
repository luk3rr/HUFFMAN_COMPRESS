/*
* Filename: parser.cc
* Created on: July  4, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include "parser.h"
#include "huffman_compress.h" // Incluído aqui devido ao problema de dependência circular

namespace huff {
    bool Parser::IsUTF8(std::ifstream &file) {
        unsigned char buffer[20];
        file.read((char*) buffer, sizeof(buffer));

        // Verifica se os bytes indicam um arquivo UTF-8 válido (sem BOM)
        int numBytes = 0;
        for (int i = 0; i < file.gcount(); i++) {
            if ((buffer[i] & 0x80) == 0)
                numBytes = 1; // Caractere ASCII de 1 byte

            else if ((buffer[i] & 0xE0) == 0xC0)
                numBytes = 2; // Inicia sequência de 2 bytes

            else if ((buffer[i] & 0xF0) == 0xE0)
                numBytes = 3; // Inicia sequência de 3 bytes

            else if ((buffer[i] & 0xF8) == 0xF0)
                numBytes = 4; // Inicia sequência de 4 bytes (máximo em UTF-8)

            else
                return false; // Byte inválido

            for (int j = 1; j < numBytes; j++) {
                if (i + j >= file.gcount()) {
                    return false; // Fim prematuro do arquivo
                }
                if ((buffer[i + j] & 0xC0) != 0x80) {
                    return false; // Caractere mal formado
                }
            }

            i += numBytes - 1;
        }
        return true;
    }

    bool Parser::CheckSignature(std::ifstream &file) {
        char buffer[SIGNATURE.size()];
        file.read(buffer, sizeof(buffer));

        return std::string(buffer, SIGNATURE.size()) == SIGNATURE;
    }

    bool Parser::CheckDecodeCompatibility(std::string filename) {
        std::ifstream file(filename, std::ios::binary);

        if (not file.is_open() or std::filesystem::is_directory(filename))
            throw huffexcpt::CouldNotOpenFile(filename);

        file.seekg(0, std::ifstream::end);

        if (file.tellg() == 0)
            throw huffexcpt::FileIsEmpty(filename);

        file.close();
        return true;
    }

    bool Parser::CheckEncodeCompatibility(std::string filename) {
        std::ifstream file(filename, std::ios::binary);

        if (not file.is_open() or std::filesystem::is_directory(filename))
            throw huffexcpt::CouldNotOpenFile(filename);

        if (not Parser::IsUTF8(file))
            throw huffexcpt::IsNotUTF8Encoding(filename);

        file.seekg(0, std::ifstream::end);

        // Verifica se o arquivo está vazio
        if (file.tellg() == 0)
            throw huffexcpt::FileIsEmpty(filename);

        file.close();
        return true;
    }
}
