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

        unsigned char* bufferRead = new unsigned char[BUFFER_MAX_SIZE];

        while (string.read((char*) bufferRead, BUFFER_MAX_SIZE) or string.gcount() > 0) {
            for (unsigned int i = 0; i < string.gcount(); i++) {

                byte = bufferRead[i];
                bits = std::bitset<8>(byte);
                byteSet.clear();

                if ((byte & ONE_BYTE_UTF8_MASK) == ONE_BYTE_UTF8_MASK) { // 0xxxxxxx -> Deve ler 1 byte
                    byteSet = bits.to_string();
                }
                else if ((byte & TWO_BYTE_UTF8_MASK) == TWO_BYTE_UTF8_MASK) { // 110xxxxx -> Deve ler 2 bytes
                    byteSet = bits.to_string();

                    i++;
                    if (i >= string.gcount()) string >> std::noskipws >> byte;
                    else byte = bufferRead[i];

                    bits = std::bitset<8>(byte);
                    byteSet += bits.to_string();
                }
                else if ((byte & THREE_BYTE_UTF8_MASK) == THREE_BYTE_UTF8_MASK) { // 1110xxxx -> Deve ler 3 bytes
                    byteSet = bits.to_string();

                    for (int j = 0; j < 2; j++) {
                        i++;
                        if (i >= string.gcount()) string >> std::noskipws >> byte;
                        else byte = bufferRead[i];

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();
                    }
                }
                else if ((byte & FOUR_BYTE_UTF8_MASK) == FOUR_BYTE_UTF8_MASK) { // 11110xxx -> Deve ler 4 bytes
                    byteSet = bits.to_string();

                    for (int j = 0; j < 3; j++) {
                        i++;
                        if (i >= string.gcount()) string >> std::noskipws >> byte;
                        else byte = bufferRead[i];

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();
                    }
                }

                if (not byteSet.empty()) {
                    map.Contains(byteSet) ? map[byteSet]++ : map[byteSet];
                }
            }
        }
        delete[] bufferRead;
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

        // Reserva o 4 bytes do arquivo
        // 1 byte para representar quantos bits do último byte são válidos
        // 3 bytes para representar o tamanho do cabeçalho
        unsigned char reservedBytes[4] = {0, 0, 0, 0};
        file.write((char*) reservedBytes, sizeof(reservedBytes));

        std::streampos headerStartPos = file.tellp();

        std::string headerBuffer;
        this->WriteTrie(file, this->m_trie.GetRoot(), headerBuffer);

        if (headerBuffer.size() % 8 != 0) { // Sobrou bits para serem gravados, completa com 1 é grava
            while (headerBuffer.size() % 8 != 0) {
                headerBuffer += "0";
            }
            this->WriteBuffer(file, headerBuffer);
        }

        // Calcula o tamanho do cabeçalho
        std::streampos headerEndPos = file.tellp();
        std::streamoff headerSize = headerEndPos - headerStartPos;

        // Escreve o tamanho do cabeçalho no arquivo
        // O primeiro byte reservado será escrito posteriormente
        file.seekp(static_cast<std::streamoff>(signatureEndPos) + 1);
        file.put((headerSize >> 16) & BYTE_MASK);
        file.put((headerSize >> 8) & BYTE_MASK);
        file.put(headerSize & BYTE_MASK);

        return headerEndPos;
    }

    void Compress::WriteTrie(std::ofstream &file, Node<std::string> *node, std::string &buffer) {
        if (node->IsLeaf()) {
            // bit 1 -> Nó folha
            buffer += "1";
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

    void Compress::Encode(std::string filename) {
        Parser::CheckEncodeCompatibility(filename);

        std::ifstream file(filename, std::ios::binary);

        if (not file.is_open())
            throw huffexcpt::CouldNotOpenFile(filename);

        map::Map<std::string, unsigned int> map;

        // Inicio de medição do tempo total da compressão
        auto encodeTime = std::chrono::high_resolution_clock::now();

        // Medição do tempo de execução do cálculo das frequências
        auto start = std::chrono::high_resolution_clock::now();

        this->Frequencies(file, map);

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Cálculo das Frequências: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - start) << std::endl;

        // Medição do tempo de execução da construção da árvore
        start = std::chrono::high_resolution_clock::now();

        this->BuildTrie(map);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "Construção da trie: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - start) << std::endl;

        // Medição do tempo de execução da construção dos códigos
        start = std::chrono::high_resolution_clock::now();

        this->BuildCode();

        end = std::chrono::high_resolution_clock::now();
        std::cout << "Construção dos códigos: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - start) << std::endl;

        file.clear();
        file.seekg(0, std::ios::beg);

        unsigned char byte;
        std::string byteSet;
        std::bitset<8> bits;
        std::string bufferWrite;

        std::filesystem::path filePath(filename);
        std::string outputFile = filePath.parent_path() / (filePath.stem().string() + filePath.extension().string() + ".bin");
        std::ofstream output(outputFile, std::ios::binary);

        if (not output.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFile);

        unsigned char* bufferRead = new unsigned char[BUFFER_MAX_SIZE];

        // Medição do tempo de compressão do arquivo
        start = std::chrono::high_resolution_clock::now();
        if (output.is_open()) {
            // Escreve o cabeçalho do arquivo
            output.seekp(this->WriteHeader(output));

            // Inicia a escrita dos dados codificiados
            while (file.read((char*) bufferRead, BUFFER_MAX_SIZE) or file.gcount() > 0) {

                for (unsigned int i = 0; i < file.gcount(); i++) {
                    byte = bufferRead[i];
                    bits = std::bitset<8>(byte);
                    byteSet.clear();

                    if ((byte & ONE_BYTE_UTF8_MASK) == ONE_BYTE_UTF8_MASK) { // 0xxxxxxx -> Deve ler 1 byte
                        byteSet = bits.to_string();
                    }
                    else if ((byte & TWO_BYTE_UTF8_MASK) == TWO_BYTE_UTF8_MASK) { // 110xxxxx -> Deve ler 2 bytes
                        byteSet = bits.to_string();

                        i++;
                        if (i >= file.gcount()) file >> std::noskipws >> byte;
                        else byte = bufferRead[i];

                        bits = std::bitset<8>(byte);
                        byteSet += bits.to_string();
                    }
                    else if ((byte & THREE_BYTE_UTF8_MASK) == THREE_BYTE_UTF8_MASK) { // 1110xxxx -> Deve ler 3 bytes
                        byteSet = bits.to_string();

                        for (int j = 0; j < 2; j++) {
                            i++;
                            if (i >= file.gcount()) file >> std::noskipws >> byte;
                            else byte = bufferRead[i];

                            bits = std::bitset<8>(byte);
                            byteSet += bits.to_string();
                        }
                    }
                    else if ((byte & FOUR_BYTE_UTF8_MASK) == FOUR_BYTE_UTF8_MASK) { // 11110xxx -> Deve ler 4 bytes
                        byteSet = bits.to_string();

                        for (int j = 0; j < 3; j++) {
                            i++;
                            if (i >= file.gcount()) file >> std::noskipws >> byte;
                            else byte = bufferRead[i];

                            bits = std::bitset<8>(byte);
                            byteSet += bits.to_string();
                        }
                    }

                    bufferWrite += this->m_map[byteSet];
                }

                if (bufferWrite.size() >= BUFFER_MAX_SIZE)
                    this->WriteBuffer(output, bufferWrite);
            }

            if (not bufferWrite.empty()) {
                // Completa o último byte e o grava no arquivo
                unsigned int junkBitsOnLastbyte = 0;
                if (bufferWrite.size() % 8 != 0) {
                    while (bufferWrite.size() % 8 != 0) {
                        bufferWrite += "1";
                        junkBitsOnLastbyte++;
                    }
                }
                this->WriteBuffer(output, bufferWrite);

                // Grava quantos bits são inválidos no último byte
                output.seekp(SIGNATURE.size(), std::ios::beg);
                unsigned char byte = junkBitsOnLastbyte;
                output.write((char*) &byte, sizeof(byte));
            }

            output.close();
            file.close();

            delete[] bufferRead;

            end = std::chrono::high_resolution_clock::now();
            std::cout << "Compressão do arquivo: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - start) << std::endl;
            std::cout << "Tempo total: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - encodeTime) << std::endl;
        }
    }

    unsigned int Compress::ReadHeader(std::ifstream &file, std::string filename) {
        // Volta a posição de leitura para o inicio do arquivo
        file.seekg(0, std::ios::beg);

        if (not Parser::CheckSignature(file))
            throw huffexcpt::InvalidSignature(filename);

        // Lê quantos bits do último byte são inválidos
        unsigned char junkBitsOnLastByte;
        file.read((char*) &junkBitsOnLastByte, sizeof(junkBitsOnLastByte));

        // Lê o tamano do cabeçalho
        unsigned char headerSizeBytes[3];
        file.read((char*) headerSizeBytes, 3);

        unsigned int headerSize = 0;
        headerSize |= headerSizeBytes[0];
        headerSize <<= 8;
        headerSize |= headerSizeBytes[1];
        headerSize <<= 8;
        headerSize |= headerSizeBytes[2];

        // Grava os dados do cabeçalho em um vector
        Vector<bool> headerData(headerSize * 8); // Vector com tamanho do cabaçalho em bits

        for (unsigned int i = 0; i < headerSize; ++i) {
            unsigned char byte;
            file.read((char*) &byte, sizeof(byte));

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

        // Retorna quantos bits do último byte são inválidos
        return junkBitsOnLastByte;
    }

    void Compress::Decode(std::string binFile) {
        Parser::CheckDecodeCompatibility(binFile);

        std::ifstream bin(binFile, std::ios::binary);

        std::filesystem::path filePath(binFile);

        std::string extension = filePath.extension().string();
        if (extension.length() >= 4 && extension.substr(extension.length() - 4) == ".bin")
            filePath.replace_extension("");

        std::string originalFileName = filePath.parent_path() / filePath.stem().string();
        std::string originalExtension = filePath.extension().string();

        std::string outputFileName = filePath.parent_path() / (filePath.stem().string() + "-decompressed" + originalExtension);
        std::ofstream decompress(outputFileName, std::ios::binary);

        // Obtém a posição atual
        std::streampos binSize = bin.tellg();

        if (not decompress.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFileName);

        unsigned int junkBitsOnLastByte = this->ReadHeader(bin, binFile);

        Node<std::string> *current = this->m_trie.GetRoot();

        std::bitset<8> bits;
        std::string bufferRead;
        std::string bufferWrite;
        unsigned char byte;
        unsigned int bufferPosition = 0;

        unsigned char* buffer = new unsigned char[BUFFER_MAX_SIZE];

        std::streampos endReadPosition = static_cast<std::streamoff>(binSize) - 1;

        auto decodeTime = std::chrono::high_resolution_clock::now();
        while (bin.read((char*) buffer, BUFFER_MAX_SIZE) or bin.gcount() > 0) {

            std::streampos currentReadPosition = bin.tellg();

            for (unsigned int i = 0; i < bin.gcount(); i++) {
                byte = buffer[i];
                bits = std::bitset<8>(byte);

                bufferRead += bits.to_string();

                while (bufferPosition < bufferRead.size()) {
                    // Se estiver no último byte, itera somente sobre os bits válidos
                    // Os bits inválidos (0's usados apenas para completar um byte e possibilitar a gravação do último byte)
                    // causavam um percorrimento na árvore e consequente inserção de um caractere aleatório no fim do arquivo
                    // descompactado
                    // IF (o último byte está no buffer AND está iterando no último byte AND e o último bit válido já foi lido)
                    if (currentReadPosition == endReadPosition and i == bin.gcount() - 1 and bufferPosition == bufferRead.size() - junkBitsOnLastByte)
                        break;

                    bufferRead[bufferPosition++] == '1' ? current = current->m_right : current = current->m_left;

                    if (current->IsLeaf()) {
                        bufferWrite += current->GetKey();
                        current = this->m_trie.GetRoot();

                        bufferRead = bufferRead.substr(bufferPosition);
                        bufferPosition = 0;
                    }
                }
            }

            if (bufferWrite.size() >= BUFFER_MAX_SIZE)
                this->WriteBuffer(decompress, bufferWrite);
        }

        if (not bufferWrite.empty())
            this->WriteBuffer(decompress, bufferWrite);

        delete[] buffer;
        decompress.close();
        bin.close();

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Descompressão do arquivo: " << std::fixed << std::chrono::duration_cast<std::chrono::duration<double>>(end - decodeTime) << std::endl;
    }

    Node<std::string> *Compress::RebuildTrie(std::ifstream &file, Vector<bool> &headerData, unsigned int &pos, unsigned int &numNodes) {
        if (pos + 7 >= headerData.Size())
            return nullptr;

        if (headerData[++pos]) {
            // Nó folha
            unsigned int charSize = 0;
            std::string charDecoding;

            // Possibilidades do primeiro byte
            // 0xxxxxxx -> 8  bits
            // 110xxxxx -> 16 bits
            // 1110xxxx -> 24 bits
            // 11110xxx -> 32 bits
            if (headerData[++pos] == 0) {// 0x
                charSize = 8 - 1; // subtrair a quantidade de bits já lidos
                charDecoding = "0";
            }
            else { // 11x...
                pos++; // Com certeza o próximo bit é 1
                if (headerData[++pos] == 0) { // 110x...
                    charSize = 16 - 3;
                    charDecoding = "110";
                }
                else if (headerData[++pos] == 0) { // 1110x..
                    charSize = 24 - 4;
                    charDecoding = "1110";
                }
                else { // 11110x...
                    pos++;
                    charSize = 32 - 5;
                    charDecoding = "11110";
                }
            }

            if (pos + charSize <= headerData.Size()) { // Ainda existem dados do cabeçalho para serem lidos

                // Armazena a sequência de bits em uma string
                for (unsigned int j = 0; j < charSize; j++)
                    headerData[++pos] ? charDecoding += "1" : charDecoding += "0";

                // Nó folha
                // Por default a frequência de cada caractere é 0 (não tem essa informação na reconstrução da trie)
                numNodes++;
                return new Node<std::string>(charDecoding, 0);
            }
        }
        else {
            // Nó interno
            numNodes++;
            Node<std::string> *leftChild = this->RebuildTrie(file, headerData, pos, numNodes);
            Node<std::string> *rightChild = this->RebuildTrie(file, headerData, pos, numNodes);

            return new Node<std::string>("", 0, leftChild, rightChild);
        }

        return nullptr;
    }

}
