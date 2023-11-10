#!/usr/bin/env python3

# File: benchmark.py
# Created on: July  4, 2023
# Author: Lucas Araújo <araujolucas@dcc.ufmg.br>

import os
import glob
import subprocess
import time
import sys
import hashlib
import matplotlib.pyplot as plt

# Realiza uma bateria de testes e plota alguns gráficos :)

MEGABYTE = 1024 * 1024

FILE_INPUT_MAX_SIZE = 10 # em MB
AMOUNT_OF_INPUTS = 4

GAP_BETWEEN_FILE_INPUT_SIZE = FILE_INPUT_MAX_SIZE / AMOUNT_OF_INPUTS

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
HUFF_COMPRESS_DIR = os.path.abspath(os.path.join(CURRENT_DIR, "../../"))
BENCHMARK_DIR = os.path.join(HUFF_COMPRESS_DIR, "test/benchmark/")
EXE_FILE = os.path.join(HUFF_COMPRESS_DIR, "bin/program")

INPUT_DIR = BENCHMARK_DIR + "inputs/"
PNG_DIR = BENCHMARK_DIR + "graphics/"
DAT_DIR = BENCHMARK_DIR + "dat/"

dirs = [INPUT_DIR, PNG_DIR, DAT_DIR]

VERIFICATION_RESULT_FILE = BENCHMARK_DIR + "tests.txt"

COMP_TIMES_FILE = DAT_DIR + "compression_times.dat"
DECOMP_TIMES_FILE = DAT_DIR + "decompression_times.dat"
COMP_RATE_FILE = DAT_DIR + "compression_rate.dat"

COMP_PNG_FILE = PNG_DIR + "compression_times.png"
DECOMP_PNG_FILE = PNG_DIR + "decompression_times.png"
RATE_PNG_FILE = PNG_DIR + "compression_rates.png"


def RemoveFiles(rmFiles):
    """
    Brief:
        Deleta todos os arquivos em um array
    Args:
        rmFiles: Lista de arquivos que serão deletados
    """
    for file in rmFiles:
        try:
            os.remove(file)
        except FileNotFoundError:
            pass

def CreateNecessaryDirs():
    """
    Brief
        Cria os diretórios necessários para os testes
    """
    for directory in dirs:
        if not os.path.exists(directory):
            os.makedirs(directory)

def GenInputFiles():
    """
    Brief
        Gera os arquivos necessários para os testes, caso eles não estejam disponíveis
    """
    print("Gerando os arquivos faltantes...")
    for i in range(1, AMOUNT_OF_INPUTS):
        filesize = i * GAP_BETWEEN_FILE_INPUT_SIZE
        filepath = os.path.join(INPUT_DIR, f"random-{filesize:.1f}MB.txt")

        if not os.path.isfile(filepath):
            subprocess.call(["python3", str(BENCHMARK_DIR + "gen_file.py"), str(filesize)], stdout=subprocess.DEVNULL)

        sys.stdout.write(f"Progresso: {i * 100 / AMOUNT_OF_INPUTS}%")
        sys.stdout.flush()
        sys.stdout.write('\r')

def CheckIntegrity(originalFile, decompressedFile):
    """
    Brief:
        Verifica a integridade do arquivo descompactado
    Args:
        originalFile: Arquivo original que foi compactado
        decompressedFile: Arquivo que foi descompactado
    """
    with open(VERIFICATION_RESULT_FILE, 'a') as f:
        if os.path.isfile(originalFile) and os.path.isfile(decompressedFile):
            original_hash = hashlib.sha256()
            decompressed_hash = hashlib.sha256()


            with open(originalFile, "rb") as fileObj:
                original_data = fileObj.read()
                original_hash.update(original_data)

            with open(decompressedFile, "rb") as fileObj:
                decompressed_data = fileObj.read()
                decompressed_hash.update(decompressed_data)

            if original_hash.digest() == decompressed_hash.digest():
                result = f"{os.path.basename(originalFile)}: PASS (Size: {os.path.getsize(originalFile) // MEGABYTE} MB)"
                passed = True
            else:
                result = f"{os.path.basename(originalFile)}: FAIL"
                passed = False

            f.write(result + "\n")

            return passed

def RunTests():
    """
    Brief
        Realiza os testes de compressão, descompressão e taxa de compressão
    """

    # Remove arquivos de testes anteriores
    rmFiles = [
        COMP_TIMES_FILE,
        DECOMP_TIMES_FILE,
        COMP_RATE_FILE,
        COMP_PNG_FILE,
        DECOMP_PNG_FILE,
        RATE_PNG_FILE,
        VERIFICATION_RESULT_FILE,
    ]

    rmFiles.extend(glob.glob(INPUT_DIR + "/*decompressed*"))
    rmFiles.extend(glob.glob(INPUT_DIR + "/*.bin"))

    RemoveFiles(rmFiles)

    print("Realizando os testes de compressão, descompressão e taxa de compressão...")

    files = sorted(os.listdir(INPUT_DIR), key=lambda f: os.path.getsize(os.path.join(INPUT_DIR, f)))

    for i, file in enumerate(files):
        filePath = os.path.join(INPUT_DIR, file)
        fileSize = os.path.getsize(filePath) // MEGABYTE

        t0 = time.time()
        subprocess.run([EXE_FILE, "-c", filePath], stdout=subprocess.DEVNULL)
        t1 = time.time()

        execution_time = t1 - t0
        with open(COMP_TIMES_FILE, "a") as f:
            f.write(f"{fileSize} {execution_time}\n")

        binFile = filePath + ".bin"
        binSize = os.path.getsize(binFile) // MEGABYTE

        compressionRate = (fileSize - binSize) / fileSize * 100
        with open(COMP_RATE_FILE, "a") as f:
            f.write(f"{fileSize} {compressionRate}\n")

        t0 = time.time()
        subprocess.run([EXE_FILE, "-d", binFile], stdout=subprocess.DEVNULL)
        t1 = time.time()

        execution_time = t1 - t0
        with open(DECOMP_TIMES_FILE, "a") as f:
            f.write(f"{fileSize} {execution_time}\n")

        progress = (i + 1) * 100 / len(files)
        sys.stdout.write(f"Progresso: {progress:.2f}%")
        sys.stdout.flush()
        sys.stdout.write('\r')

        decompressedFile = os.path.join(INPUT_DIR, filePath.replace(".txt", "-decompressed.txt"))

        passed = CheckIntegrity(filePath, decompressedFile)

        if passed:
            os.remove(binFile)
            os.remove(decompressedFile)

def PlotAnalysis():
    data = zip(*[map(float, line.split()) for line in open(COMP_TIMES_FILE)])
    plt.plot(*data, marker="o")
    plt.xlabel("Tamanho do arquivo (MB)")
    plt.ylabel("Tempo para a compressão (s)")
    plt.savefig(COMP_PNG_FILE)
    plt.close()

    data = zip(*[map(float, line.split()) for line in open(DECOMP_TIMES_FILE)])
    plt.plot(*data, marker="o")
    plt.xlabel("Tamanho do arquivo (MB)")
    plt.ylabel("Tempo para a descompressão (s)")
    plt.savefig(DECOMP_PNG_FILE)
    plt.close()

    data = zip(*[map(float, line.split()) for line in open(COMP_RATE_FILE)])
    plt.plot(*data, marker="o")
    plt.xlabel("Tamanho do arquivo original (MB)")
    plt.ylabel("Taxa de compressão (%)")
    plt.savefig(RATE_PNG_FILE)
    plt.close()

def Main():
    CreateNecessaryDirs()
    GenInputFiles()
    RunTests()
    PlotAnalysis()

if __name__ == "__main__":
    Main()
