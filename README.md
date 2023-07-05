# HUFFMAN_COMPRESS
HUFFMAN_COMPRESS é um programa que possibilita a compressão e descompressão de arquivos codificados como UTF-8. Esse programa é o produto de uma atividade prática da disciplina de Estruturas de Dados do [Departamento de Ciência da Computação da UFMG](https://dcc.ufmg.br/).

## Sumário
- [Compilação](#Compilação)
- [Execução](#Execução)
- [Documentação](#Documentação)

# Compilação
HUFFMAN_COMPRESS pode ser compilado com o seguinte comando:

``` sh
$ make build
```

OBS.: O programa foi desenvolvido e testado utilizando o g++12. Certifique-se de utilizar esta versão do compilador.

# Execução
A execução pode ser realizada tanto pelo Makefile: ``make run ARGS="<params>"``

quanto pela execução direta do executável: ``bin/program <params>``

Os parâmetros disponíveis seguem abaixo:

| Parâmetro                   | Descrição             |
|-----------------------------|-----------------------|
| `-c, --compress <file>`     | Compacta o arquivo    |
| `-d, --decompress <binary>` | Descompacta o binário |
| `-h, --help`                | Mensagem de ajuda     |

- A compressão de um arquivo gerará um arquivo no mesmo diretório do arquivo original, mas com a extensão ``.bin``.
- A descompactação produzirá um arquivo no mesmo diretório do arquivo binário, com o nome e extensão presentes no nome do binário.

OBS.: A descompactação só pode ser realizada em arquivos compactados por este programa. Implementações diferentes do algoritmo de Huffman produzem binários diferentes.

# Documentação
A primeira versão da documentação bem como o enunciado deste trabalho pode ser lida [aqui](https://github.com/luk3rr/HUFFMAN_COMPRESS/tree/main/docs).
