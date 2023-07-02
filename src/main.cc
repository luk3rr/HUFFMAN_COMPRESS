/*
* Filename: main.cc
* Created on: June 28, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include <cstdlib>
#include "huffman_compress.h"
#include <exception>
#include <iostream>
#include <string>
#include <getopt.h>

void PrintUsage() {
    std::cout << "Huffman Compress" << std::endl;
    std::cout << "Opções:" << std::endl;
    std::cout << "  -c, --compress    Comprimir o arquivo" << std::endl;
    std::cout << "  -d, --decompress  Descomprimir o arquivo" << std::endl;
    std::cout << "  -h, --help        Exibir esta mensagem de ajuda" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string filePath;

    const char* const shortOptions = "c:d:h";
    const option longOptions[] = {
        {"compress", required_argument, nullptr, 'c'},
        {"decompress", required_argument, nullptr, 'd'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int option;
    int optionIndex = -1;
    bool compress = false;
    bool decompress = false;

    while ((option = getopt_long(argc, argv, shortOptions, longOptions, &optionIndex)) != -1) {
        switch (option) {
            case 'c':
                compress = true;
                filePath = optarg;
                break;
            case 'd':
                decompress = true;
                filePath = optarg;
                break;
            case 'h':
                PrintUsage();
                return EXIT_SUCCESS;
            default:
                PrintUsage();
                return EXIT_FAILURE;
        }
    }

    if (compress and decompress) {
        std::cout << "As opções -c/--compress e -d/--decompress são mutuamente exclusivas." << std::endl;
        return EXIT_FAILURE;
    }

    std::string outputFileName;
    huff::Compress compressor;

    if (compress) {
        try {
            compressor.Encode(filePath);
            std::cout << "Compressão concluída!" << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if (decompress) {
        try {
            compressor.Decode(filePath);
            std::cout << "Descompressão concluída!" << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else {
        std::cout << "Nenhuma opção selecionada. Use -c/--compress para compressão ou -d/--decompress para descompressão." << std::endl;
        PrintUsage();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
