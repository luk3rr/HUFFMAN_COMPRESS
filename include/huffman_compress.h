/*
* Filename: huffman_compress.h
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#ifndef HUFFMAN_COMPRESS_H_
#define HUFFMAN_COMPRESS_H_

#include <fstream>
#include <string>
#include <bitset>
#include <exception>
#include <ios>
#include <chrono>
#include <string>
#include <filesystem>
#include <cstddef>

#include "huffman_trie.h"
#include "huffman_compress_excpt.h"
#include "map_node.h"
#include "node_trie.h"
#include "priority_queue_min.h"
#include "map.h"
#include "vector.h"
#include "parser.h"

const std::string SIGNATURE = "HUFF"; // Assinatura do arquivo comprimido
const std::size_t BUFFER_MAX_SIZE = 1024 * 16; // 16 KB

namespace huff {
    class Compress {
        private:
            Trie m_trie;
            map::Map<std::string, std::string> m_map;

            /**
             * @brief Calcula a frequência de ocorrências de cada caractere da string
             * @param string String que será utilizada no cálculo
             * @param map Map com as frequências de cada caractere
             **/
            void Frequencies(std::ifstream &string, map::Map<std::string, unsigned int> &map);

            /**
             * @brief Constroi a Trie de Huffman
             * @param map Map com as frequências de cada caractere
             **/
            void BuildTrie(map::Map<std::string, unsigned int> &map);

            /**
             * @brief Cria o código dos caracteres
             **/
            void BuildCode();

            /**
             * @brief Cria o código dos caracteres (chamada recursiva)
             * @param node Nó atual
             * @param code Código até o momento
             **/
            void BuildCode(Node<std::string> *node, std::string code);

            /**
             * @brief Escreve os bits no arquivo
             * @param file Arquivo que será escrito
             * @param buffer Buffer com os bits que serão gravados
             **/
            void WriteBuffer(std::ofstream &file, std::string &buffer);

            /**
             * @brief Escreve os dados para a decodificação no cabeçalho do arquivo binário
             * @param file Arquivo no qual ocorrerá a escrita
             **/
            std::streampos WriteHeader(std::ofstream &file);

            /**
             * @brief Lê o cabeçalho do arquivo binário
             * @param file Arquivo binário que será lido
             * @return Quantos bits do último byte do arquivo são válidos
             **/
            unsigned int ReadHeader(std::ifstream &file, std::string filename);

            /**
             * @brief Escreve a informação para a reconstrução da árvore no arquivo binário
             * @param file Arquivo no qual ocorrerá a escrita
             * @param node Nó atual da chamada recursiva
             * @param buffer Buffer com os bits que serão gravados
             **/
            void WriteTrie(std::ofstream &file, Node<std::string> *node, std::string &buffer);

            /**
             * @brief Reconstroí a trie a partir das informações gravadas no arquivo binário
             * @param file Arquivo binário que será lido
             * @param headerData Vector com os bits do cabeçalho do arquivo
             * @param pos Posição atual no vector
             * @param numNodes Número de nós atualmente na árvore
             **/
            Node<std::string> *RebuildTrie(std::ifstream &file, Vector<bool> &headerData, unsigned int &pos, unsigned int &numNodes);

        public:
            Compress();

            ~Compress();

            /**
             * @brief Realiza a compressão do arquivo
             * @param file Arquivo que será comprimido
             **/
            void Encode(std::string file);

            /**
             * @brief Realiza a descompressão do arquivo
             * @param file Arquivo que será descomprimido
             **/
            void Decode(std::string file);
    };
}

#endif // HUFFMAN_COMPRESS_H_
