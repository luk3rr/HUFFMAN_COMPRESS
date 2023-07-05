#!/usr/bin/env python3

# Filename: gen_file.py
# Created on: July  4, 2023
# Author: Lucas Araújo <araujolucas@dcc.ufmg.br>

import random
import string
import sys

MEGABYTE = 1024 ** 2
BUFFER_MAX_SIZE = MEGABYTE
WORD_MAX_LENGTH = 20

def WriteBuffer(filename, buffer):
    with open(filename, 'a', encoding='utf-8') as file:
        file.write(buffer)

# filesize em MB
def GenTextFile(filesize):
    totalsize = filesize * MEGABYTE
    filename = "random-" + str(filesize) + "MB.txt"
    words = []
    currentSize = 0
    bufferSize = 0

    while currentSize < totalsize:
        word_length = random.randint(1, WORD_MAX_LENGTH)
        word = ''.join(random.choices(string.ascii_letters, k=word_length))
        words.append(word)
        bufferSize += word_length + 1  # Adicionar 1 para o espaço em branco
        currentSize += word_length + 1

        if bufferSize >= BUFFER_MAX_SIZE:
            encoded_text = ' '.join(words)
            WriteBuffer(filename, encoded_text + ' ')

            words = []
            bufferSize = 0

    if words:
        encoded_text = ' '.join(words)
        WriteBuffer(filename, encoded_text)

def main():
    if len(sys.argv) < 2:
        print("Especifique o tamanho em MB como argumento.")
        print("Exemplo: python3 script.py 1")

    else:
        try:
            filesizeMB = float(sys.argv[1])
            GenTextFile(filesizeMB)
            print("Arquivo gerado")

        except ValueError:
            print("O tamanho deve ser um número válido em MB.")

if __name__ == "__main__":
    main()
