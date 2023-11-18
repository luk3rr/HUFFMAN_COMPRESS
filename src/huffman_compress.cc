/*
 * Filename: huffman_compress.cc
 * Created on: June 27, 2023
 * Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
 */

#include "huffman_compress.h"
#include "pair.h"
#include "priority_queue_bheap.h"
#include <cstddef>
#include <cstdint>
#include <string>

namespace huff
{
    Compress::Compress() { }

    Compress::~Compress() { }

    // TODO: Adicionar execeção para quando a leitura do byte no arquivo não puder ser
    // feita ex.: 1110xx... No fim do arquivo -> Tentativa de ler mais bytes
    void Compress::Frequencies(std::ifstream&                         string,
                               rbtree::Map<std::string, std::size_t>& map)
    {
        unsigned char          byte;
        std::string            byteSet;
        std::bitset<BYTE_SIZE> bits;

        // Buffer de leitura para otimizar o processo de leitura dos bits e contagem
        // dos caracteres
        unsigned char* bufferRead = new unsigned char[BUFFER_MAX_SIZE];

        while (string.read((char*)bufferRead, BUFFER_MAX_SIZE) or string.gcount() > 0)
        {
            for (std::size_t i = 0; i < string.gcount(); i++)
            {

                byte = bufferRead[i];
                bits = std::bitset<BYTE_SIZE>(byte);
                byteSet.clear();

                if ((byte & ONE_BYTE_UTF8_MASK) == ONE_BYTE_UTF8_MASK)
                { // 0xxxxxxx -> Deve ler 1 byte
                    byteSet = bits.to_string();
                }
                else if ((byte & TWO_BYTE_UTF8_MASK) == TWO_BYTE_UTF8_MASK)
                { // 110xxxxx -> Deve ler 2 bytes
                    byteSet = bits.to_string();

                    i++;
                    if (i >= string.gcount())
                        string >> std::noskipws >> byte;
                    else
                        byte = bufferRead[i];

                    bits = std::bitset<BYTE_SIZE>(byte);
                    byteSet += bits.to_string();
                }
                else if ((byte & THREE_BYTE_UTF8_MASK) == THREE_BYTE_UTF8_MASK)
                { // 1110xxxx -> Deve ler 3 bytes
                    byteSet = bits.to_string();

                    for (int j = 0; j < 2; j++)
                    {
                        i++;
                        if (i >= string.gcount())
                            string >> std::noskipws >> byte;
                        else
                            byte = bufferRead[i];

                        bits = std::bitset<BYTE_SIZE>(byte);
                        byteSet += bits.to_string();
                    }
                }
                else if ((byte & FOUR_BYTE_UTF8_MASK) == FOUR_BYTE_UTF8_MASK)
                { // 11110xxx -> Deve ler 4 bytes
                    byteSet = bits.to_string();

                    for (int j = 0; j < 3; j++)
                    {
                        i++;
                        if (i >= string.gcount())
                            string >> std::noskipws >> byte;
                        else
                            byte = bufferRead[i];

                        bits = std::bitset<BYTE_SIZE>(byte);
                        byteSet += bits.to_string();
                    }
                }

                if (not byteSet.empty())
                {
                    // Incrementa o contador de frequência
                    map[byteSet]++;
                }
            }
        }

        delete[] bufferRead;
    }

    void Compress::BuildTrie(rbtree::Map<std::string, std::size_t>& map)
    {
        bheap::PriorityQueue<dlkd::Node<TrieInfo>*> pqueue;

        for (auto& pair : map)
        {
            pqueue.Enqueue(
                new dlkd::Node<TrieInfo>(TrieInfo(pair.GetFirst(), pair.GetSecond())));
        }

        dlkd::Node<TrieInfo>* x;
        dlkd::Node<TrieInfo>* y;

        std::size_t freqSum = 0;

        while (pqueue.Size() > 1)
        {
            x = pqueue.Dequeue();
            y = pqueue.Dequeue();

            freqSum = x->GetValue().GetFrequencie() + y->GetValue().GetFrequencie();

            pqueue.Enqueue(new dlkd::Node<TrieInfo>(TrieInfo("", freqSum), x, y));
        }

        this->m_trie.InsertExistingTree(pqueue.Dequeue(), map.Size());
    }

    void Compress::BuildCode()
    {
        this->BuildCode(this->m_trie.GetRoot(), "");
    }

    void Compress::BuildCode(dlkd::Node<TrieInfo>* node, std::string code)
    {

        // Check if node is an leaf
        if (not(node->GetLeftNode() or node->GetRightNode()))
        {
            this->m_map.Insert(node->GetValue().GetBits(), code);
            return;
        }

        this->BuildCode(node->GetLeftNode(), code + "0");
        this->BuildCode(node->GetRightNode(), code + "1");
    }

    void Compress::WriteBuffer(std::ofstream& file, std::string& buffer)
    {
        if (not buffer.empty() and file.is_open())
        {
            std::size_t bufferSize = buffer.size();
            std::size_t numBytes   = bufferSize / BYTE_SIZE;

            unsigned char* data = new unsigned char[numBytes];

            // Passa os bits na string para um array
            for (std::size_t i = 0; i < numBytes; ++i)
            {
                unsigned char byte = 0;

                for (std::size_t j = 0; j < BYTE_SIZE; ++j)
                {
                    if (buffer[i * BYTE_SIZE + j] == '1')
                        byte |= (1 << (BYTE_SIZE - 1 - j));
                }
                data[i] = byte;
            }

            // Escreve os dados do buffer no arquivo
            file.write((char*)data, numBytes);

            // Remove os dados que foram gravados do buffer
            buffer = buffer.substr(numBytes * BYTE_SIZE);

            delete[] data;
        }
    }

    std::streampos Compress::WriteHeader(std::ofstream& file)
    {
        // Volta a posição de escrita para o inicio do arquivo
        file.seekp(0, std::ios::beg);

        // Escreve a assinatura do programa nos primeiros bytes do arquivo
        file.write(SIGNATURE.data(), SIGNATURE.size());
        std::streampos signatureEndPos = file.tellp();

        // Reserva os bytes iniciais do arquivo do arquivo
        unsigned char reservedBytes[HEADER_RESERVED_BYTES_AT_START] = {};

        file.write((char*)reservedBytes, sizeof(reservedBytes));

        std::streampos headerStartPos = file.tellp();

        std::string headerBuffer;
        this->WriteTrie(file, this->m_trie.GetRoot(), headerBuffer);

        if (headerBuffer.size() % BYTE_SIZE != 0)
        { // Sobrou bits para serem gravados, completa com 0 é grava
            while (headerBuffer.size() % BYTE_SIZE != 0)
            {
                headerBuffer += "0";
            }
            this->WriteBuffer(file, headerBuffer);
        }

        // Calcula o tamanho do cabeçalho
        std::streampos headerEndPos = file.tellp();
        std::streamoff headerSize   = headerEndPos - headerStartPos;

        // Escreve o tamanho do cabeçalho no arquivo
        // O primeiro byte reservado será escrito posteriormente
        file.seekp(static_cast<std::streamoff>(signatureEndPos) + 1);
        file.put((headerSize >> BYTE_SIZE * 2) & BYTE_MASK);
        file.put((headerSize >> BYTE_SIZE) & BYTE_MASK);
        file.put(headerSize & BYTE_MASK);

        return headerEndPos;
    }

    void Compress::WriteTrie(std::ofstream&        file,
                             dlkd::Node<TrieInfo>* node,
                             std::string&          buffer)
    {
        // Check if node is an leaf
        if (not(node->GetLeftNode() or node->GetRightNode()))
        {
            // bit 1 -> Nó folha
            buffer += "1";
            buffer += node->GetValue().GetBits();
            this->WriteBuffer(file, buffer);
        }
        else
        {
            // bit 0 -> Nó interno
            buffer += "0";
            WriteTrie(file, node->GetLeftNode(), buffer);
            WriteTrie(file, node->GetRightNode(), buffer);
        }
    }

    void Compress::Encode(std::string filename)
    {
        Parser::CheckEncodeCompatibility(filename);

        std::ifstream file(filename, std::ios::binary);

        if (not file.is_open())
            throw huffexcpt::CouldNotOpenFile(filename);

        rbtree::Map<std::string, std::size_t> map;

        // Inicio de medição do tempo total da compressão
        auto encodeTime = std::chrono::high_resolution_clock::now();

        // Medição do tempo de execução do cálculo das frequências
        auto start = std::chrono::high_resolution_clock::now();

        this->Frequencies(file, map);

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Cálculo das Frequências: " << std::fixed
                  << std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                               start)
                  << std::endl;

        // Medição do tempo de execução da construção da árvore
        start = std::chrono::high_resolution_clock::now();

        this->BuildTrie(map);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "Construção da trie: " << std::fixed
                  << std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                               start)
                  << std::endl;

        // Medição do tempo de execução da construção dos códigos
        start = std::chrono::high_resolution_clock::now();

        this->BuildCode();

        end = std::chrono::high_resolution_clock::now();
        std::cout << "Construção dos códigos: " << std::fixed
                  << std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                               start)
                  << std::endl;

        file.clear();
        file.seekg(0, std::ios::beg);

        unsigned char          byte;
        std::string            byteSet;
        std::bitset<BYTE_SIZE> bits;
        std::string            bufferWrite;

        std::filesystem::path filePath(filename);
        std::string           outputFile =
            filePath.parent_path() /
            (filePath.stem().string() + filePath.extension().string() + ".bin");
        std::ofstream output(outputFile, std::ios::binary);

        if (not output.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFile);

        unsigned char* bufferRead = new unsigned char[BUFFER_MAX_SIZE];

        // Medição do tempo de compressão do arquivo
        start = std::chrono::high_resolution_clock::now();
        if (output.is_open())
        {
            // Escreve o cabeçalho do arquivo
            output.seekp(this->WriteHeader(output));

            // Inicia a escrita dos dados codificiados
            while (file.read((char*)bufferRead, BUFFER_MAX_SIZE) or file.gcount() > 0)
            {

                for (std::size_t i = 0; i < file.gcount(); i++)
                {
                    byte = bufferRead[i];
                    bits = std::bitset<BYTE_SIZE>(byte);
                    byteSet.clear();

                    if ((byte & ONE_BYTE_UTF8_MASK) == ONE_BYTE_UTF8_MASK)
                    { // 0xxxxxxx -> Deve ler 1 byte
                        byteSet = bits.to_string();
                    }
                    else if ((byte & TWO_BYTE_UTF8_MASK) == TWO_BYTE_UTF8_MASK)
                    { // 110xxxxx -> Deve ler 2 bytes
                        byteSet = bits.to_string();

                        i++;
                        if (i >= file.gcount())
                            file >> std::noskipws >> byte;
                        else
                            byte = bufferRead[i];

                        bits = std::bitset<BYTE_SIZE>(byte);
                        byteSet += bits.to_string();
                    }
                    else if ((byte & THREE_BYTE_UTF8_MASK) == THREE_BYTE_UTF8_MASK)
                    { // 1110xxxx -> Deve ler 3 bytes
                        byteSet = bits.to_string();

                        for (int j = 0; j < 2; j++)
                        {
                            i++;
                            if (i >= file.gcount())
                                file >> std::noskipws >> byte;
                            else
                                byte = bufferRead[i];

                            bits = std::bitset<BYTE_SIZE>(byte);
                            byteSet += bits.to_string();
                        }
                    }
                    else if ((byte & FOUR_BYTE_UTF8_MASK) == FOUR_BYTE_UTF8_MASK)
                    { // 11110xxx -> Deve ler 4 bytes
                        byteSet = bits.to_string();

                        for (int j = 0; j < 3; j++)
                        {
                            i++;
                            if (i >= file.gcount())
                                file >> std::noskipws >> byte;
                            else
                                byte = bufferRead[i];

                            bits = std::bitset<BYTE_SIZE>(byte);
                            byteSet += bits.to_string();
                        }
                    }

                    bufferWrite += this->m_map[byteSet];
                }

                if (bufferWrite.size() >= BUFFER_MAX_SIZE)
                    this->WriteBuffer(output, bufferWrite);
            }

            if (not bufferWrite.empty())
            {
                // Completa o último byte e o grava no arquivo
                std::size_t junkBitsOnLastbyte = 0;
                if (bufferWrite.size() % BYTE_SIZE != 0)
                {
                    while (bufferWrite.size() % BYTE_SIZE != 0)
                    {
                        bufferWrite += "1";
                        junkBitsOnLastbyte++;
                    }
                }
                this->WriteBuffer(output, bufferWrite);

                // Grava quantos bits são inválidos no último byte
                output.seekp(SIGNATURE.size(), std::ios::beg);
                unsigned char byte = junkBitsOnLastbyte;
                output.write((char*)&byte, sizeof(byte));
            }

            output.close();
            file.close();

            delete[] bufferRead;

            end = std::chrono::high_resolution_clock::now();
            std::cout << "Compressão do arquivo: " << std::fixed
                      << std::chrono::duration_cast<std::chrono::duration<double>>(
                             end - start)
                      << std::endl;
            std::cout << "Tempo total: " << std::fixed
                      << std::chrono::duration_cast<std::chrono::duration<double>>(
                             end - encodeTime)
                      << std::endl;
        }
    }

    std::size_t Compress::ReadHeader(std::ifstream& file, std::string filename)
    {
        // Volta a posição de leitura para o inicio do arquivo
        file.seekg(0, std::ios::beg);

        if (not Parser::CheckSignature(file))
            throw huffexcpt::InvalidSignature(filename);

        // Lê quantos bits do último byte são inválidos
        unsigned char junkBitsOnLastByte;
        file.read((char*)&junkBitsOnLastByte, sizeof(junkBitsOnLastByte));

        // Lê o tamano do cabeçalho
        unsigned char headerSizeBytes[HEADER_SIZE_IN_BYTES];
        file.read((char*)headerSizeBytes, HEADER_SIZE_IN_BYTES);

        std::size_t headerSize = 0;
        // HEADER_SIZE_BYTEs - 1, pois o último byte não deve sofrer shift
        for (std::size_t i = 0; i < HEADER_SIZE_IN_BYTES - 1; i++)
        {
            headerSize |= headerSizeBytes[i];
            headerSize <<= BYTE_SIZE;
        }

        headerSize |= headerSizeBytes[HEADER_SIZE_IN_BYTES - 1];

        // Grava os dados do cabeçalho em um vector
        // Vector com tamanho do cabaçalho em bits
        Vector<bool> headerData(headerSize);

        for (std::size_t i = 0; i < headerSize; ++i)
        {
            unsigned char byte;
            file.read((char*)&byte, sizeof(byte));

            // Armazena cada bit em uma posição do vector
            for (int j = BYTE_SIZE - 1; j >= 0; j--)
            {
                bool bit = (byte >> j) & 1;
                headerData.PushBack(bit);
            }
        }

        // Reconstroí a Huffman trie
        std::size_t           numNodes = 0; // Número de nós na árvore
        std::size_t           pos      = 0; // Posição no vector
        dlkd::Node<TrieInfo>* root     = new dlkd::Node<TrieInfo>(TrieInfo("root", 0));
        root->SetLeftNode(this->RebuildTrie(file, headerData, pos, numNodes));
        root->SetRightNode(this->RebuildTrie(file, headerData, pos, numNodes));

        // Armazena a árvore reconstruída
        this->m_trie.DeleteTree();
        this->m_trie.InsertExistingTree(root, numNodes);

        // Retorna quantos bits do último byte são inválidos
        return junkBitsOnLastByte;
    }

    void Compress::Decode(std::string binFile)
    {
        Parser::CheckDecodeCompatibility(binFile);

        std::ifstream bin(binFile, std::ios::binary);

        std::filesystem::path filePath(binFile);

        std::string extension = filePath.extension().string();
        if (extension.length() >= 4 &&
            extension.substr(extension.length() - 4) == ".bin")
            filePath.replace_extension("");

        std::string originalFileName =
            filePath.parent_path() / filePath.stem().string();
        std::string originalExtension = filePath.extension().string();

        std::string outputFileName =
            filePath.parent_path() /
            (filePath.stem().string() + "-decompressed" + originalExtension);
        std::ofstream decompress(outputFileName, std::ios::binary);

        // Obtém a posição atual
        std::streampos binSize = bin.tellg();

        if (not decompress.is_open())
            throw huffexcpt::CouldNotOpenFile(outputFileName);

        std::size_t junkBitsOnLastByte = this->ReadHeader(bin, binFile);

        dlkd::Node<TrieInfo>* current = this->m_trie.GetRoot();

        std::bitset<BYTE_SIZE> bits;
        std::string            bufferRead;
        std::string            bufferWrite;
        unsigned char          byte;
        std::size_t            bufferPosition = 0;

        unsigned char* buffer = new unsigned char[BUFFER_MAX_SIZE];

        std::streampos endReadPosition = static_cast<std::streamoff>(binSize) - 1;

        auto decodeTime = std::chrono::high_resolution_clock::now();
        while (bin.read((char*)buffer, BUFFER_MAX_SIZE) or bin.gcount() > 0)
        {

            std::streampos currentReadPosition = bin.tellg();

            for (std::size_t i = 0; i < bin.gcount(); i++)
            {
                byte = buffer[i];
                bits = std::bitset<BYTE_SIZE>(byte);

                bufferRead += bits.to_string();

                while (bufferPosition < bufferRead.size())
                {
                    // Se estiver no último byte, itera somente sobre os bits válidos
                    // Os bits inválidos (0's usados apenas para completar um byte e
                    // possibilitar a gravação do último byte) causavam um percorrimento
                    // na árvore e consequente inserção de um caractere aleatório no fim
                    // do arquivo descompactado IF (o último byte está no buffer AND
                    // está iterando no último byte AND e o último bit válido já foi
                    // lido)
                    if (currentReadPosition == endReadPosition and
                        i == bin.gcount() - 1 and
                        bufferPosition == bufferRead.size() - junkBitsOnLastByte)
                        break;

                    bufferRead[bufferPosition++] == '1'
                        ? current = current->GetRightNode()
                        : current = current->GetLeftNode();

                    // Check if node is an leaf
                    if (not(current->GetLeftNode() or current->GetRightNode()))
                    {
                        bufferWrite += current->GetValue().GetBits();
                        current = this->m_trie.GetRoot();

                        bufferRead     = bufferRead.substr(bufferPosition);
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
        std::cout << "Descompressão do arquivo: " << std::fixed
                  << std::chrono::duration_cast<std::chrono::duration<double>>(
                         end - decodeTime)
                  << std::endl;
    }

    dlkd::Node<TrieInfo>* Compress::RebuildTrie(std::ifstream& file,
                                                Vector<bool>&  headerData,
                                                std::size_t&   pos,
                                                std::size_t&   numNodes)
    {
        if (pos + 7 >= headerData.Size())
            return nullptr;

        if (headerData[++pos])
        {
            // Nó folha
            std::size_t charSize = 0;
            std::string charDecoding;

            // Possibilidades do primeiro byte
            // 0xxxxxxx -> 8  bits
            // 110xxxxx -> 16 bits
            // 1110xxxx -> 24 bits
            // 11110xxx -> 32 bits
            if (headerData[++pos] == 0)
            {                                 // 0x
                charSize     = BYTE_SIZE - 1; // subtrair a quantidade de bits já lidos
                charDecoding = "0";
            }
            else
            {          // 11x...
                pos++; // Com certeza o próximo bit é 1
                if (headerData[++pos] == 0)
                { // 110x...
                    charSize     = BYTE_SIZE * 2 - 3;
                    charDecoding = "110";
                }
                else if (headerData[++pos] == 0)
                { // 1110x..
                    charSize     = BYTE_SIZE * 3 - 4;
                    charDecoding = "1110";
                }
                else
                { // 11110x...
                    pos++;
                    charSize     = BYTE_SIZE * 4 - 5;
                    charDecoding = "11110";
                }
            }

            if (pos + charSize <= headerData.Size())
            { // Ainda existem dados do cabeçalho para serem lidos

                // Armazena a sequência de bits em uma string
                for (std::size_t j = 0; j < charSize; j++)
                    headerData[++pos] ? charDecoding += "1" : charDecoding += "0";

                // Nó folha
                // Por default a frequência de cada caractere é 0 (não tem essa
                // informação na reconstrução da trie)
                numNodes++;
                return new dlkd::Node<TrieInfo>(TrieInfo(charDecoding, 0));
            }
        }
        else
        {
            // Nó interno
            numNodes++;
            dlkd::Node<TrieInfo>* leftChild =
                this->RebuildTrie(file, headerData, pos, numNodes);
            dlkd::Node<TrieInfo>* rightChild =
                this->RebuildTrie(file, headerData, pos, numNodes);

            return new dlkd::Node<TrieInfo>(TrieInfo("", 0), leftChild, rightChild);
        }

        return nullptr;
    }

} // namespace huff
