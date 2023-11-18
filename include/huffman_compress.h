/*
 * Filename: huffman_compress.h
 * Created on: June 27, 2023
 * Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
 */

#ifndef HUFFMAN_COMPRESS_H_
#define HUFFMAN_COMPRESS_H_

#include <bitset>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

#include "binary_tree.h"
#include "huffman_compress_excpt.h"
#include "map.h"
#include "node.h"
#include "node_rbtree.h"
#include "parser.h"
#include "priority_queue_bheap.h"
#include "vector.h"

// Máscaras UTF-8
constexpr uint8_t ONE_BYTE_UTF8_MASK   = 0x00; // 00000000
constexpr uint8_t TWO_BYTE_UTF8_MASK   = 0xC0; // 11000000
constexpr uint8_t THREE_BYTE_UTF8_MASK = 0xE0; // 11100000
constexpr uint8_t FOUR_BYTE_UTF8_MASK  = 0xF0; // 11110000
constexpr uint8_t BYTE_MASK            = 0xFF; // 11111111

// Assinatura do arquivo comprimido
inline constexpr std::string_view SIGNATURE       = "HUFF";
constexpr uint32_t                BUFFER_MAX_SIZE = 1024 * 16; // 16 kB
constexpr uint8_t                 BYTE_SIZE       = 8;         // Um byte, oito bits

// Número de bytes reservados no início do arquivo
// 1 byte para representar a quantidade de bits válidos no último byte
// 3 bytes para representar o tamanho do cabeçalho
constexpr uint8_t HEADER_RESERVED_BYTES_AT_START = 4;
constexpr uint8_t HEADER_SIZE_IN_BYTES           = 3;

namespace huff
{
    class TrieInfo
    {
        private:
            std::size_t m_frequencie;
            std::string m_bits;

        public:
            TrieInfo(const std::string bits, const std::size_t frequecie)
                : m_bits(bits),
                  m_frequencie(frequecie)
            { }

            std::size_t GetFrequencie() const
            {
                return m_frequencie;
            }

            std::string GetBits() const
            {
                return m_bits;
            }

            // Função amiga para o overload do operador <<
            friend std::ostream& operator<<(std::ostream& os, const TrieInfo& trieInfo)
            {
                os << "Bits: " << trieInfo.m_bits
                   << ", Frequencie: " << trieInfo.m_frequencie;
                return os;
            }
    };

    class Compress
    {
        private:
            rbtree::Map<std::string, std::string> m_map;

            BinaryTree<TrieInfo> m_trie;

            /**
             * @brief Calcula a frequência de ocorrências de cada caractere da string
             * @param string String que será utilizada no cálculo
             * @param map Map com as frequências de cada caractere
             **/
            void Frequencies(std::ifstream&                         string,
                             rbtree::Map<std::string, std::size_t>& map);

            /**
             * @brief Constroi a Trie de Huffman
             * @param map Map com as frequências de cada caractere
             **/
            void BuildTrie(rbtree::Map<std::string, std::size_t>& map);

            /**
             * @brief Cria o código dos caracteres
             **/
            void BuildCode();

            /**
             * @brief Cria o código dos caracteres (chamada recursiva)
             * @param node Nó atual
             * @param code Código até o momento
             **/
            void BuildCode(dlkd::Node<TrieInfo>* node, std::string code);

            /**
             * @brief Escreve os bits no arquivo
             * @param file Arquivo que será escrito
             * @param buffer Buffer com os bits que serão gravados
             **/
            void WriteBuffer(std::ofstream& file, std::string& buffer);

            /**
             * @brief Escreve os dados para a decodificação no cabeçalho do arquivo
             *binário
             * @param file Arquivo no qual ocorrerá a escrita
             **/
            std::streampos WriteHeader(std::ofstream& file);

            /**
             * @brief Lê o cabeçalho do arquivo binário
             * @param file Arquivo binário que será lido
             * @return Quantos bits do último byte do arquivo são inválidos
             **/
            std::size_t ReadHeader(std::ifstream& file, std::string filename);

            /**
             * @brief Escreve a informação para a reconstrução da árvore no arquivo
             *binário
             * @param file Arquivo no qual ocorrerá a escrita
             * @param node Nó atual da chamada recursiva
             * @param buffer Buffer com os bits que serão gravados
             **/
            void WriteTrie(std::ofstream&        file,
                           dlkd::Node<TrieInfo>* node,
                           std::string&          buffer);

            /**
             * @brief Reconstroí a trie a partir das informações gravadas no arquivo
             *binário
             * @param file Arquivo binário que será lido
             * @param headerData Vector com os bits do cabeçalho do arquivo
             * @param pos Posição atual no vector
             * @param numNodes Número de nós atualmente na árvore
             **/
            dlkd::Node<TrieInfo>* RebuildTrie(std::ifstream& file,
                                              Vector<bool>&  headerData,
                                              std::size_t&   pos,
                                              std::size_t&   numNodes);

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
} // namespace huff

#endif // HUFFMAN_COMPRESS_H_
