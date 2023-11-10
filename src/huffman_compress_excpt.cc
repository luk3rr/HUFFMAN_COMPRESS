/*
 * Filename: huffman_compress_excpt.cc
 * Created on: June 30, 2023
 * Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
 */

#include "huffman_compress_excpt.h"

huffexcpt::CouldNotOpenFile::CouldNotOpenFile(std::string file)
{
    this->m_msg = "ERRO: Falha ao abrir o arquivo '" + file + "'";
}

const char* huffexcpt::CouldNotOpenFile::what() const throw()
{
    return this->m_msg.c_str();
}

huffexcpt::IsNotUTF8Encoding::IsNotUTF8Encoding(std::string file)
{
    this->m_msg = "ERRO: O arquivo '" + file + "' não está codificado como UTF-8";
}

const char* huffexcpt::IsNotUTF8Encoding::what() const throw()
{
    return this->m_msg.c_str();
}

huffexcpt::InvalidSignature::InvalidSignature(std::string file)
{
    this->m_msg = "ERRO: Falha na assinatura. O arquivo '" + file +
                  "' não pode ser descompactado.";
}

const char* huffexcpt::InvalidSignature::what() const throw()
{
    return this->m_msg.c_str();
}

huffexcpt::FileIsEmpty::FileIsEmpty(std::string file)
{
    this->m_msg = "ERRO: O arquivo '" + file + "' está vazio.";
}

const char* huffexcpt::FileIsEmpty::what() const throw()
{
    return this->m_msg.c_str();
}
