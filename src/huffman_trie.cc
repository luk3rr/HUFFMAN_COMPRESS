/*
* Filename: huffman_trie.cc
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include "huffman_trie.h"

namespace huff {
    Trie::Trie() {
        this->m_root = nullptr;
        this->m_nodes = 0;
    }

    Trie::~Trie() {
        this->DeleteTree();
    }

    Node<std::string> *Trie::GetRoot() {
        return this->m_root;
    }

    void Trie::DeleteTree() {
        this->DeleteTree(this->m_root);
        this->m_root = nullptr;
    }

    void Trie::DeleteTree(Node<std::string> *node) {
        if (node != nullptr) {
            this->DeleteTree(node->m_left);
            this->DeleteTree(node->m_right);
            delete node;
        }
    }

    void Trie::InsertExistingTree(Node<std::string> *root, int nodes) {
        if (this->m_root != nullptr)
            throw bntexcpt::TreeIsNotEmpty();

        if (root == nullptr or nodes < 1)
            throw bntexcpt::NewTreeIsEmpty();

        this->m_nodes = nodes + 1;
        this->m_root = root;
    }


    void Trie::InorderTreeWalk(Vector<std::string> &walk) {
        if (this->m_root != nullptr) {
            this->InorderTreeWalk(walk, this->m_root->m_left);
            walk.PushBack(this->m_root->m_key);
            this->InorderTreeWalk(walk, this->m_root->m_right);
        }
    }

    void Trie::InorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node) {
        if (node != nullptr) {
            this->InorderTreeWalk(walk, node->m_left);
            walk.PushBack(node->m_key);
            this->InorderTreeWalk(walk, node->m_right);
        }
    }

    void Trie::PreorderTreeWalk(Vector<std::string> &walk) {
        if (this->m_root != nullptr) {
            walk.PushBack(this->m_root->m_key);
            this->PreorderTreeWalk(walk, this->m_root->m_left);
            this->PreorderTreeWalk(walk, this->m_root->m_right);
        }
    }

    void Trie::PreorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node) {
        if (node != nullptr) {
            walk.PushBack(node->m_key);
            this->PreorderTreeWalk(walk, node->m_left);
            this->PreorderTreeWalk(walk, node->m_right);
        }
    }

    void Trie::PostorderTreeWalk(Vector<std::string> &walk) {
        if (this->m_root != nullptr) {
            this->PostorderTreeWalk(walk, this->m_root->m_left);
            this->PostorderTreeWalk(walk, this->m_root->m_right);
            walk.PushBack(this->m_root->m_key);
        }
    }

    void Trie::PostorderTreeWalk(Vector<std::string> &walk, Node<std::string> *node) {
        if (node != nullptr) {
            this->PostorderTreeWalk(walk, node->m_left);
            this->PostorderTreeWalk(walk, node->m_right);
            walk.PushBack(node->m_key);
        }
    }

    void Trie::LevelorderTreeWalk() {
        Vector<Node<std::string>*> aux;
        Node<std::string> *node;

        aux.PushBack(this->m_root);

        while (!aux.IsEmpty()) {
            node = aux.PopBack();
            if (node != nullptr) {
                std::cout << node->m_key << " ";
                aux.PushBack(node->m_left);
                aux.PushBack(node->m_right);
            }
        }
    }

    void Trie::LevelorderTreeWalk(Vector<std::string> &walk) {
        Vector<Node<std::string>*> aux;
        Node<std::string> *node;

        aux.PushBack(this->m_root);

        while (!aux.IsEmpty()) {
            node = aux.PopBack();
            if (node != nullptr) {
                walk.PushBack(node->m_key);
                aux.PushBack(node->m_left);
                aux.PushBack(node->m_right);
            }
        }
    }

    void Trie::DumpTree(std::ofstream &output) {
        if (this->m_root != nullptr) {
            output << this->m_root->m_key
                   << " : " << this->m_root->m_freq
                   << std::endl;
            this->DumpTree(this->m_root->m_left, "", output, true);
            this->DumpTree(this->m_root->m_right, "", output, false);
        }
    }

    void Trie::DumpTree(Node<std::string> *&node, const std::string &vBar, std::ofstream &output, bool sideIsLeft) {
        if (node != nullptr) {
            if (sideIsLeft) {
                output << vBar << "├───" << node->m_key << " : " << node->m_freq << std::endl;
            }
            else {
                output << vBar << "└───" << node->m_key << " : " << node->m_freq << std::endl;
            }
            this->DumpTree(node->m_left, vBar + (sideIsLeft ? "│    " : "     "), output, true);
            this->DumpTree(node->m_right, vBar + (sideIsLeft ? "│    " : "     "), output, false);
        }
    }
}
