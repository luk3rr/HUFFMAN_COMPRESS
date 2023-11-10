/*
 * Filename: huffman_compress_excpt.h
 * Created on: June 30, 2023
 * Author: Lucas Araújo <araujolucas@dcc.ufmg.br>
 */

#ifndef HUFFMAN_COMPRESS_EXCPT_H_
#define HUFFMAN_COMPRESS_EXCPT_H_

#include <exception>
#include <string>

namespace huffexcpt
{
    class CouldNotOpenFile : public std::exception
    {
        private:
            std::string m_msg;

        public:
            CouldNotOpenFile(std::string file);

            const char* what() const throw();
    };

    class IsNotUTF8Encoding : public std::exception
    {
        private:
            std::string m_msg;

        public:
            IsNotUTF8Encoding(std::string file);

            const char* what() const throw();
    };

    class InvalidSignature : public std::exception
    {
        private:
            std::string m_msg;

        public:
            InvalidSignature(std::string file);

            const char* what() const throw();
    };

    class FileIsEmpty : public std::exception
    {
        private:
            std::string m_msg;

        public:
            FileIsEmpty(std::string file);

            const char* what() const throw();
    };

} // namespace huffexcpt

#endif // HUFFMAN_COMPRESS_EXCPT_H_
