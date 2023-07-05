#!/usr/bin/env sh

# Filename: benchmark.sh
# Created on: July  4, 2023
# Author: Lucas Araújo <araujolucas@dcc.ufmg.br>

MEGABYTE=$((1024 * 1024))

INPUT_DIR="inputs"
EXE_FILE="$HOME/Projects/HUFF_COMPRESS/bin/program"

COMP_TIMES_FILE="compression_times.dat"
DECOMP_TIMES_FILE="decompression_times.dat"
COMP_RATE_FILE="compression_rate.dat"

COMP_PNG_FILE="compression_times.png"
DECOMP_PNG_FILE="decompression_times.png"
RATE_PNG_FILE="compression_rates.png"

rm -f "$COMP_TIMES_FILE" "$DECOMP_TIMES_FILE" "$COMP_RATE_FILE"
rm -f "$COMP_PNG_FILE" "$DECOMP_PNG_FILE" "$RATE_PNG_FILE"
rm -f "$INPUT_DIR"/*.bin

# Itere sobre os arquivos de teste
for file in $(ls -v "$INPUT_DIR"); do
  file="$INPUT_DIR/$file"

  fileSize=$((($(wc -c < "$file")) / $MEGABYTE))

  # Medição do tempo de compressão
  startTime=$(date +%s.%N)
  $EXE_FILE -c "$file"
  endTime=$(date +%s.%N)

  executionTime=$(echo "$endTime - $startTime" | bc)
  echo "$fileSize $executionTime" >> "$COMP_TIMES_FILE"

  # Cálculo da taxa de compressão
  binFile="${file}.bin"

  binSize=$((($(wc -c < "$binFile")) / $MEGABYTE))

  compressionRate=$(awk -v b="$binSize" -v f="$fileSize" 'BEGIN{printf "%.2f", (f - b) / f * 100}')

  echo "$fileSize $compressionRate" >> "$COMP_RATE_FILE"

  # Medição do tempo de descompressão
  startTime=$(date +%s.%N)
  $EXE_FILE -d "$binFile"
  endTime=$(date +%s.%N)

  executionTime=$(echo "$endTime - $startTime" | bc)
  echo "$fileSize $executionTime" >> "$DECOMP_TIMES_FILE"
done

# Gráfico tempo de compressão
gnuplot <<EOF
set xlabel "Tamanho do arquivo"
set ylabel "Tempo para compactação"
set terminal png
set output "$COMP_PNG_FILE"
plot "$COMP_TIMES_FILE" with linespoints
EOF

# Gráfico tempo de descompressão
gnuplot <<EOF
set xlabel "Tamanho do arquivo"
set ylabel "Tempo para descompactação"
set terminal png
set output "$DECOMP_PNG_FILE"
plot "$DECOMP_TIMES_FILE" with linespoints
EOF

# Gráfico taxa de compressão
gnuplot <<EOF
set xlabel "Tamanho do arquivo original"
set ylabel "Taxa de compressão"
set terminal png
set output "$RATE_PNG_FILE"
plot "$COMP_RATE_FILE" with linespoints
EOF

echo "Bateria de testes concluída!"
