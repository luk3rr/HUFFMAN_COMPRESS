/*
* Filename: huffman_compress.cc
* Created on: June 27, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include "huffman_compress.h"

namespace huff {
    Compress::Compress() { }

    Compress::~Compress() { }

    // TODO: Adicionar execeção para quanto a leitura do byte no arquivo não puder ser feita
    // ex.: 1110xx... No fim do arquivo -> Tentativa de ler mais bytes
    void Compress::Frequencies(std::ifstream &string, map::Map<std::string, unsigned int> &map) {
        unsigned char byte;
        std::string byteSet;
        std::bitset<8> bits;

        while (string >> std::noskipws >> byte) {
            bits = std::bitset<8>(byte);

            if ((byte & 0x80) == 0) { // 0xxxxxxx -> Deve ler 1 byte
                byteSet.clear();
                byteSet = bits.to_string();

                // Operadtor de incremento da seg fault em alguns casos quando a chave ainda não existe
                if (map.Contains(byteSet))
                    map[byteSet]++;
                else
                    map[byteSet];
            }
            else if ((byte & 0xE0) == 0xC0) { // 110xxxxx -> Deve ler 2 bytes
                byteSet.clear();
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;

                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                if (map.Contains(byteSet))
                    map[byteSet]++;
                else
                    map[byteSet];
            }
            else if ((byte & 0xF0) == 0xE0) { // 1110xxxx -> Deve ler 3 bytes
                byteSet.clear();
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;
                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;
                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                if (map.Contains(byteSet))
                    map[byteSet]++;
                else
                    map[byteSet];
            }
            else if ((byte & 0xF8) == 0xF0) { // 11110xxx -> Deve ler 4 bytes
                byteSet.clear();
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;
                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;
                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                string >> std::noskipws >> byte;
                bits = std::bitset<8>(byte);
                byteSet += bits.to_string();

                if (map.Contains(byteSet))
                    map[byteSet]++;
                else
                    map[byteSet];
            }
        }
    }

    void Compress::BuildTrie(map::Map<std::string, unsigned int> &map) {
        MinPQueue<std::string> pqueue;

        for (auto &pair : map) {
            pqueue.Insert(new Node<std::string>(pair.GetKey(), pair.GetValue()));
        }

        Node<std::string> *x;
        Node<std::string> *y;

        while (pqueue.Size() > 1) {
            x = pqueue.Delete();
            y = pqueue.Delete();
            pqueue.Insert(new Node<std::string>("", x->m_freq + y->m_freq, x, y));
        }

        this->m_trie.InsertExistingTree(pqueue.Delete(), map.Size());
    }

    void Compress::BuildCode() {
        this->BuildCode(this->m_trie.GetRoot(), "");
    }

    void Compress::BuildCode(Node<std::string> *node, std::string code) {
        if (node->IsLeaf()) {
            this->m_map.Insert(node->m_key, code);
            return;
        }

        this->BuildCode(node->m_left, code + "0");
        this->BuildCode(node->m_right, code + "1");
    }

    void Compress::WriteBuffer(std::ofstream& file, std::string& buffer) {
        if (!buffer.empty() and file.is_open()) {
            std::size_t bufferSize = buffer.size();
            std::size_t numBytes = bufferSize / 8;

            unsigned char* data = new unsigned char[numBytes];

            // Passa os bits na string para um array
            for (std::size_t i = 0; i < numBytes; ++i) {
                unsigned char byte = 0;

                for (std::size_t j = 0; j < 8; ++j) {
                    if (buffer[i * 8 + j] == '1')
                        byte |= (1 << (7 - j));
                }

                data[i] = byte;
            }

            // Escreve os dados do buffer no arquivo
            file.write((char*) data, numBytes);

            // Remove os dados que foram gravados do buffer
            buffer = buffer.substr(numBytes * 8);

            delete[] data;
        }
    }

    std::streampos Compress::WriteHeader(std::ofstream &file) {
        // Volta a posição de escrita para o inicio do arquivo
        file.seekp(0, std::ios::beg);

        // Escreve a assinatura do programa nos primeiros bytes do arquivo
        file.write(SIGNATURE.data(), SIGNATURE.size());
        std::streampos signatureEndPos = file.tellp();

        // Reserva os três primeiros bytes do arquivo para representar o tamanho do cabeçalho
        unsigned char reservedBytes[3] = {0, 0, 0};
        file.write(reinterpret_cast<char*>(reservedBytes), 3);

        std::streampos headerStartPos = file.tellp();

        std::string headerBuffer;
        this->WriteTrie(file, this->m_trie.GetRoot(), headerBuffer);

        if (headerBuffer.size() % 8 != 0) { // Sobrou bits para serem gravados, completa com 1 é grava
            while (headerBuffer.size() % 8 != 0) {
                headerBuffer += "1";
            }
            this->WriteBuffer(file, headerBuffer);
        }

        // Calcula o tamanho do cabeçalho
        std::streampos headerEndPos = file.tellp();
        std::streamoff headerSize = headerEndPos - headerStartPos;

        // Escreve o tamanho do cabeçalho no arquivo
        file.seekp(signatureEndPos);
        file.put((headerSize >> 16) & 0xFF);
        file.put((headerSize >> 8) & 0xFF);
        file.put(headerSize & 0xFF);

        return headerEndPos;
    }

    void Compress::WriteTrie(std::ofstream &file, Node<std::string> *node, std::string &buffer) {
        if (node->IsLeaf()) {
            // bit 1 -> Nó folha
            buffer += "1";

            // Determina a quantidade de bits que forma o caractere atual
            // A representação é a seguinte:
            // O primeiro conjunto de 0's representa se o nó é interno
            // O primeiro 1 encontrado significa que um nó folha foi encontrado
            // Os próximos dois bits são reservados para dizer quantos bits forma o caractere codificado nesse nó folha
            // e.g.: [0...] 1 [xx] [8bits, 16bits, 24bits ou 32bits]...
            // O processo reinicia...

            std::string bits = node->m_key;
            if (bits.size() == 8)
                buffer += "00";

            else if (bits.size() == 16)
                buffer += "01";

            else if (bits.size() == 24)
                buffer += "10";

            else
                buffer += "11";

            buffer += node->m_key;
            this->WriteBuffer(file, buffer);
        }
        else {
            // bit 0 -> Nó interno
            buffer += "0";

            WriteTrie(file, node->m_left, buffer);
            WriteTrie(file, node->m_right, buffer);
        }
    }

    void Compress::Encode(std::string fileName) {
        if (not this->IsUTF8(fileName))
            throw huffexcpt::IsNotUTF8Encoding(fileName);

        std::ifstream file(fileName, std::ios::binary);

        if (not file.is_open() or std::filesystem::is_directory(fileName))
            throw huffexcpt::CouldNotOpenFile(fileName);

        map::Map<std::string, unsigned int> map;

        this->Frequencies(file, map);
        this->BuildTrie(map);
        this->BuildCode();

        file.clear();
        file.seekg(0, std::ios::beg);

        unsigned char byte;
        std::string byteSet;
        std::bitset<8> bits;
        std::string bufferWrite;

        std::filesystem::path filePath(fileName);
        std::string outputFile = filePath.stem().string() + ".bin";
        std::ofstream output(outputFile, std::ios::binary);

        if (not output.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFile);

        unsigned char* bufferRead = new unsigned char[BUFFER_MAX_SIZE];

        if (output.is_open()) {
            // Escreve o cabeçalho do arquivo
            output.seekp(this->WriteHeader(output));

            // Inicia a escrita dos dados codificiados
            while (file.read((char*) bufferRead, BUFFER_MAX_SIZE) or file.gcount() > 0) {

                for (unsigned int i = 0; i < file.gcount(); i++) {
                    byte = static_cast<unsigned char>(bufferRead[i]);
                    bits = std::bitset<8>(byte);

                    if ((byte & 0x80) == 0) { // 0xxxxxxx -> Deve ler 1 byte
                        bufferWrite += this->m_map[bits.to_string()];

                    }
                    else if ((byte & 0xE0) == 0xC0) { // 110xxxxx -> Deve ler 2 bytes
                        byteSet.clear();
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        bufferWrite += this->m_map[byteSet];
                    }
                    else if ((byte & 0xF0) == 0xE0) { // 1110xxxx -> Deve ler 3 bytes
                        byteSet.clear();
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        bufferWrite += this->m_map[byteSet];
                    }
                    else if ((byte & 0xF8) == 0xF0) { // 11110xxx -> Deve ler 4 bytes
                        byteSet.clear();
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        i++;
                        if (i >= file.gcount()) // Todos os bytes do buffer já foram lidos
                            file >> std::noskipws >> byte;

                        else
                            byte = static_cast<unsigned char>(bufferRead[i]);

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();

                        bufferWrite += this->m_map[byteSet];
                    }
                }

                if (bufferWrite.size() >= BUFFER_MAX_SIZE) {
                    this->WriteBuffer(output, bufferWrite);
                }
            }

            if (not bufferWrite.empty()) {
                if (bufferWrite.size() % 8 != 0) {
                    while (bufferWrite.size() % 8 != 0) {
                        bufferWrite += "0";
                    }
                }
                this->WriteBuffer(output, bufferWrite);
            }
            output.close();
            file.close();
        }
    }

    std::streampos Compress::ReadHeader(std::ifstream &file, std::string filename) {
        // Volta a posição de leitura para o inicio do arquivo
        file.seekg(0, std::ios::beg);

        if (not this->CheckSignature(file))
            throw huffexcpt::InvalidSignature(filename);

        // Lê o tamano do cabeçalho
        unsigned char headerSizeBytes[3];
        file.read(reinterpret_cast<char*>(headerSizeBytes), 3);

        unsigned int headerSize = 0;
        headerSize |= static_cast<unsigned int>(headerSizeBytes[0]);
        headerSize <<= 8;
        headerSize |= static_cast<unsigned int>(headerSizeBytes[1]);
        headerSize <<= 8;
        headerSize |= static_cast<unsigned int>(headerSizeBytes[2]);

        // Grava os dados do cabeçalho em um vector
        Vector<bool> headerData(headerSize * 8); // Vector com tamanho do cabaçalho em bits

        for (unsigned int i = 0; i < headerSize; ++i) {
            unsigned char byte;
            file.read(reinterpret_cast<char*>(&byte), sizeof(byte));

            // Armazena cada bit em uma posição do vector
            for (int j = 7; j >= 0; j--) {
                bool bit = (byte >> j) & 1;
                headerData.PushBack(bit);
            }
        }

        // Reconstroí a Huffman trie
        unsigned int numNodes = 0; // Número de nós na árvore
        unsigned int pos = 0; // Posição no vector
        Node<std::string> *root = new Node<std::string>("root", 0);
        root->m_left = this->RebuildTrie(file, headerData, pos, numNodes);
        root->m_right = this->RebuildTrie(file, headerData, pos, numNodes);

        // Armazena a árvore reconstruída
        this->m_trie.DeleteTree();
        this->m_trie.InsertExistingTree(root, numNodes);

        return file.tellg();
    }

    void Compress::Decode(std::string binFile) {
        std::ifstream bin(binFile, std::ios::binary);

        std::filesystem::path filePath(binFile);
        std::string outputFileName = filePath.stem().string() + "-decompress.txt";
        std::ofstream decompress(outputFileName, std::ios::binary);

        if (not bin.is_open() or std::filesystem::is_directory(binFile))
            throw huffexcpt::CouldNotOpenFile(binFile);

        if (not decompress.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFileName);

        this->ReadHeader(bin, binFile);

        Node<std::string> *current = this->m_trie.GetRoot();

        std::bitset<8> bits;
        std::string bufferRead;
        std::string bufferWrite;
        unsigned char byte;
        unsigned int bufferPosition = 0;

        unsigned char* buffer = new unsigned char[BUFFER_MAX_SIZE];

        while (bin.read((char*) buffer, BUFFER_MAX_SIZE) or bin.gcount() > 0) {

            for (unsigned int i = 0; i < bin.gcount(); i++) {
                byte = static_cast<unsigned char>(buffer[i]);
                bits = std::bitset<8>(byte);

                bufferRead += bits.to_string();

                while (bufferPosition < bufferRead.size()) {
                    if (bufferRead[bufferPosition++] == '1')
                        current = current->m_right;
                    else
                        current = current->m_left;

                    if (current->IsLeaf()) {
                        bufferWrite += current->GetKey();
                        current = this->m_trie.GetRoot();

                        bufferRead = bufferRead.substr(bufferPosition);
                        bufferPosition = 0;
                    }
                }
            }

            if (bufferWrite.size() >= BUFFER_MAX_SIZE) {
                this->WriteBuffer(decompress, bufferWrite);
            }
        }

        std::cout << bufferWrite.size() << std::endl;
        if (not bufferWrite.empty()) {
            this->WriteBuffer(decompress, bufferWrite);
        }

        std::cout << bufferWrite.size() << std::endl;
        decompress.close();
        bin.close();
    }

    Node<std::string> *Compress::RebuildTrie(std::ifstream &file, Vector<bool> &headerData, unsigned int &pos, unsigned int &numNodes) {
        if (pos < headerData.Size()) {
            if (headerData[++pos]) {
                // Nó folha
                if (pos + 2 <= headerData.Size()) {
                    unsigned int charSize = 0;

                    // Verifica os próximos dois bits que dizem quantos bits formam o próximo caractere
                    if (headerData[++pos] == 0) { // 0x
                        if (headerData[++pos] == 0) charSize = 8; // 00 -> 8bits
                        else charSize = 16; // 01 -> 16bits
                    }
                    else { // 1x
                        if (headerData[++pos] == 0) charSize = 24; // 10 -> 24bits
                        else charSize = 32; // 11 -> 32bits
                    }

                    if (pos + charSize <= headerData.Size()) { // Ainda existem dados do cabeçalho para serem lidos
                        std::string charDecoding;

                        // Armazena a sequência de bits em uma string
                        for (unsigned int j = 0; j < charSize; j++)
                            headerData[++pos] ? charDecoding += "1" : charDecoding += "0";

                        // Nó folha
                        // Por default a frequência de cada caractere é 0 (não tem essa informação na reconstrução da trie)
                        numNodes++;
                        return new Node<std::string>(charDecoding, 0);
                    }
                }
            }
            else {
                // Nó interno
                numNodes++;
                Node<std::string> *leftChild = this->RebuildTrie(file, headerData, pos, numNodes);
                Node<std::string> *rightChild = this->RebuildTrie(file, headerData, pos, numNodes);

                return new Node<std::string>("", 0, leftChild, rightChild);
            }
        }

        return nullptr;
    }

    bool Compress::IsUTF8(std::string filename) {
        std::ifstream file(filename, std::ios::binary);

        if (not file.is_open())
            throw huffexcpt::CouldNotOpenFile(filename);

        char buffer[4];
        file.read(buffer, sizeof(buffer));

        // Verifica se os primeiros bytes indicam um arquivo UTF-8 válido
        if (file.gcount() >= 3) {
            if (buffer[0] == char(0xEF) and buffer[1] == char(0xBB) and buffer[2] == char(0xBF))
                return true; // UTF-8 com BOM
        }

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

    bool Compress::CheckSignature(std::ifstream &file) {
        char buffer[SIGNATURE.size()];
        file.read(buffer, sizeof(buffer));

        return std::string(buffer, SIGNATURE.size()) == SIGNATURE;
    }
}
