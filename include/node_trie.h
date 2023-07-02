/*
* Filename: node_trie.h
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#ifndef NODE_TRIE_H_
#define NODE_TRIE_H_

namespace huff {
    template<typename typeT>
    class Node {
        public:
            typeT m_key;
            unsigned int m_freq;
            Node<typeT> *m_left;
            Node<typeT> *m_right;
            Node<typeT> *m_parent;

            Node(typeT key, unsigned int freq);

            Node(typeT key, unsigned int freq, Node<typeT> *left, Node<typeT> *right);

            typeT GetKey();
            bool IsLeaf();
    };

    template<typename typeT>
    Node<typeT>::Node(typeT key, unsigned int freq) {
        this->m_key = key;
        this->m_freq = freq;
        this->m_left = this->m_right = this->m_parent = nullptr;
    }

    template<typename typeT>
    Node<typeT>::Node(typeT key, unsigned int freq, Node<typeT> *left, Node<typeT> *right) {
        this->m_key = key;
        this->m_freq = freq;
        this->m_left = left;
        this->m_right = right;
        this->m_parent = nullptr;
    }

    template<typename typeT>
    bool Node<typeT>::IsLeaf() {
        return (this->m_left == nullptr) and (this->m_right == nullptr);
    }

    template<typename typeT>
    typeT Node<typeT>::GetKey() {
        return this->m_key;
    }
}

#endif // NODE_TRIE_H_
