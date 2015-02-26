IR
==

Information Retrieval

Este projeto é referente ao trabalho final da disciplina de RI.

Consiste em uma máquina de busca para pesquisas textuais e de imagem para uma coleção de uma loja de moda.

Foi desenvolvido em C++ e Python.

Requesitos:
  . Compilador C++
  . Python 2.7
  . Biblioteca Pillow
  
** Antes de executar, é necessário editar o arquivo setup.properties para configurar os caminhos das imagens e dos arquivos xml que serão indexados.

======== Pesquisa Textutal ========

Executar o arquivo "pesquisaTextual.sh". Caso deseja ver o log no modo debug, basta passar -V como argumento:
  ./pesquisaTextual.sh -V
  
======== Pesquisa Imagem ==========  

Executar o arquivo "pesquisaImagem.sh". Caso deseja ver o log no modo debug, basta passar -V como argumento:
  ./pesquisaImagem.sh -V
  
** A pesquisa por imagem necessita de, no mínmo, 25gb de espaço livre.
