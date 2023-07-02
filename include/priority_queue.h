/*
* Filename: priority_queue.h
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#ifndef PRIORITY_QUEUE_H_
#define PRIORITY_QUEUE_H_

#include "node_trie.h"

// Classe para implementação das filas de prioridade mínima e máxima

template<typename typeT>
class PriorityQueue {
    public:
        virtual void Insert(huff::Node<typeT> *newNode) = 0;

        virtual huff::Node<typeT> *Get() = 0;

        virtual huff::Node<typeT> *Delete() = 0;

        virtual bool IsEmpty() = 0;

        virtual unsigned int Size() = 0;
};

#endif // PRIORITY_QUEUE_H_
