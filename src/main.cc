/*
* Filename: main.cc
* Created on: June 28, 2023
* Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
*/

#include "huffman_compress.h"
#include <exception>
#include <iostream>
#include <string>
#include <getopt.h>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::string output;

    huff::Compress compressor;

    if (argc < 4 or (not (std::string(argv[1]) == "-c" or std::string(argv[1]) == "-d"))) {
        std::cout << "Use:" << std::endl;
        std::cout << "-c <arquivo-que-sera-comprimido> <nome-arquivo-saida>" << std::endl;
        std::cout << "-d <arquivo-que-sera-descomprimido> <nome-arquivo-saida>" << std::endl;
        return EXIT_FAILURE;
    }

    if (std::string(argv[1]) == "-c") {
        std::string fileToEncode = std::string(argv[2]);
        output = std::string(argv[3]);

        try {

            compressor.Encode(fileToEncode, output);

            std::filesystem::path inputPath(fileToEncode);
            std::ifstream in(fileToEncode, std::ios::binary);
            std::ifstream out(output, std::ios::binary);

            if (in.is_open() and out.is_open()) {
                in.seekg(0, std::ios::end);
                std::streampos inputSize = in.tellg();

                out.seekg(0, std::ios::end);
                std::streampos outputSize = out.tellg();

                if (inputSize > outputSize) {
                    double compressionRate = ((inputSize - outputSize) / static_cast<double>(inputSize)) * 100;
                    std::cout << std::fixed << std::setprecision(2);
                    std::cout << "Taxa de compressão: " << compressionRate << "%" << std::endl;
                }

                in.close();
                out.close();
            }
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    else if (std::string(argv[1]) == "-d") {
        try {
            std::string fileToDecode = std::string(argv[2]);
            output = std::string(argv[3]);
            compressor.Decode(fileToDecode, output);
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
