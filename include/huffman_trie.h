/*
* Filename: huffman_trie.h
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#ifndef HUFFMAN_TRIE_H_
#define HUFFMAN_TRIE_H_

#include <fstream>
#include <iostream>

#include "node_trie.h"
#include "binary_tree_excpt.h"
#include "vector.h"

// Huffman namespace
namespace huff {
    class Trie {
        private:
            Node<std::string> *m_root;
            int m_nodes;

            /**
            @brief Deleta toda a árvore
            @param node Nó atual que será deletado a cada chamada recursiva da função
            */
            void DeleteTree(Node<std::string> *node);

            /**
            @brief Realiza o caminhamento in-ordem na árvore
            @param node Ponteiro para o próximo
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void InorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node);

            /**
            @brief Realiza o caminhamento pós-ordem na árvore
            @param node Ponteiro para o próximo
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void PostorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node);

            /**
            @brief Realiza o caminhamento pré-ordem na árvore
            @param node Ponteiro para o próximo
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void PreorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node);

            /**
            @brief Imprime a árvore
            @param output Arquivo em que a impressão será feita
            @param node Nó atual
            @param side, deve receber True se o nó está o lado esquerdo ou False, caso o nó esteja do lado direito
            */
            void DumpTree(Node<std::string> *&node, const std::string &vBar, std::ofstream &output, bool sideIsLeft);

        public:
            Trie();

            ~Trie();

            Node<std::string> *GetRoot();

            /**
            @brief Deleta toda a árvore
            */
            void DeleteTree();

            void InsertExistingTree(Node<std::string> *root, int nodes);

            /**
            @brief Realiza o caminhamento in-ordem na árvore
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void InorderTreeWalk(Vector<std::string> &walk);

            /**
            @brief Realiza o caminhamento pré-ordem na árvore
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void PreorderTreeWalk(Vector<std::string> &walk);

            /**
            @brief Realiza o caminhamento pós-ordem na árvore
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void PostorderTreeWalk(Vector<std::string> &walk);

            /**
            @brief Realiza o caminhamento por nível na árvore e vai printando os elementos
            */
            void LevelorderTreeWalk();

            /**
            @brief Realiza o caminhamento por nível na árvore
            @param walk Fila em que serão armazenados os itens do caminhamento
            */
            void LevelorderTreeWalk(Vector<std::string> &walk);

            /**
            @brief Imprime a árvore
            @param output Arquivo em que a impressão será feita
            */
            void DumpTree(std::ofstream &output);

    };
}

#endif // HUFFMAN_TRIE_H_
