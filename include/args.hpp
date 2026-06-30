#ifndef ARGS_HPP
#define ARGS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file args.hpp
 * @brief Definições de estruturas e funções para manipulação de argumentos de linha de comando.
 */

/**
 * @struct Parameters
 * @brief Estrutura que armazena os parâmetros de execução passados via CLI para a JVM.
 */
typedef struct {
    char* class_file_path;  /**< Caminho para o arquivo .class de entrada */
    char* output_file_path; /**< Caminho para o arquivo de saída do relatório textual (-o=) */
    bool  execute;          /**< Flag indicando modo de operação: true para execução (-e), false para leitura (-r) */
} Parameters;

/**
 * @brief Analisa os argumentos passados via linha de comando para inicializar os parâmetros da JVM.
 * 
 * @param argc Número de argumentos passados na linha de comando.
 * @param argv Vetor de strings contendo os argumentos.
 * @return Parameters Estrutura preenchida com os caminhos e a opção de modo de execução.
 */
const Parameters analisar_args(int argc, char* argv[]);

/**
 * @brief Valida se os parâmetros fornecidos atendem aos requisitos mínimos de execução da JVM.
 * 
 * @param params Ponteiro para a estrutura Parameters a ser validada.
 * @return true Se os parâmetros forem válidos (ex: caminho presente e arquivo de saída especificado no modo leitor).
 * @return false Se faltarem parâmetros obrigatórios ou houver conflito nas opções.
 */
bool validar_parametros(const Parameters* params);

#endif

