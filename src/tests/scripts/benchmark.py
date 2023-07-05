#!/usr/bin/env python3

# File: benchmark.py
# Created on: July  4, 2023
# Author: Lucas Araújo <araujolucas@dcc.ufmg.br>

import os
import glob
import subprocess
import time
import hashlib
import matplotlib.pyplot as plt

# Realiza uma bateria de testes e plota alguns gráficos :)

MEGABYTE = 1024 * 1024

BENCHMARK_DIR = os.path.expanduser("~/Projects/HUFF_COMPRESS/src/tests/scripts/")
EXE_FILE = os.path.expanduser("~/Projects/HUFF_COMPRESS/bin/program")
INPUT_DIR = BENCHMARK_DIR + "inputs/"
PNG_DIR = BENCHMARK_DIR + "graphics/"
DAT_DIR = BENCHMARK_DIR + "dat/"

TEST_OUTPUT_FILE = BENCHMARK_DIR + "tests.txt"

COMP_TIMES_FILE = DAT_DIR + "compression_times.dat"
DECOMP_TIMES_FILE = DAT_DIR + "decompression_times.dat"
COMP_RATE_FILE = DAT_DIR + "compression_rate.dat"

COMP_PNG_FILE = PNG_DIR + "compression_times.png"
DECOMP_PNG_FILE = PNG_DIR + "decompression_times.png"
RATE_PNG_FILE = PNG_DIR + "compression_rates.png"

files_to_remove = [
    COMP_TIMES_FILE,
    DECOMP_TIMES_FILE,
    COMP_RATE_FILE,
    COMP_PNG_FILE,
    DECOMP_PNG_FILE,
    RATE_PNG_FILE,
    TEST_OUTPUT_FILE,
]

files_to_remove.extend(glob.glob(INPUT_DIR + "/*decompressed*"))
files_to_remove.extend(glob.glob(INPUT_DIR + "/*.bin"))

for file in files_to_remove:
    try:
        os.remove(file)
    except FileNotFoundError:
        pass

# Pega os arquivos no diretório por ordem de tamanho em bytes
for file in sorted(os.listdir(INPUT_DIR), key=lambda f: os.path.getsize(os.path.join(INPUT_DIR, f))):
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

with open(TEST_OUTPUT_FILE, "w") as f:
    for filename in sorted(os.listdir(INPUT_DIR), key=lambda f: os.path.getsize(os.path.join(INPUT_DIR, f))):
        if filename.endswith(".txt"):
            original_file = os.path.join(INPUT_DIR, filename)
            decompressed_file = os.path.join(INPUT_DIR, filename.replace(".txt", "-decompressed.txt"))

            if os.path.isfile(original_file) and os.path.isfile(decompressed_file):
                original_hash = hashlib.sha256()
                decompressed_hash = hashlib.sha256()

                with open(original_file, "rb") as fileObj:
                    original_data = fileObj.read()
                    original_hash.update(original_data)

                with open(decompressed_file, "rb") as fileObj:
                    decompressed_data = fileObj.read()
                    decompressed_hash.update(decompressed_data)

                if original_hash.digest() == decompressed_hash.digest():
                    result = f"{os.path.basename(filename)}: PASS (Size: {os.path.getsize(original_file)} bytes)"
                else:
                    result = f"{os.path.basename(filename)}: FAIL"

                f.write(result + "\n")

# Plota os gráficos
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

print("Test battery completed!")
