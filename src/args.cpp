#include "args.hpp"
#include <iostream>

using namespace std;

/**
 * @brief Extrai o valor associado a uma flag específica nos argumentos de linha de comando.
 *
 * Realiza a comparação de prefixo na string do argumento para identificar flags passadas
 * no formato "-r=", "-o=" ou "-e=" e retorna o ponteiro para o início do valor após o sinal de igual.
 *
 * @param arg Argumento bruto sendo inspecionado.
 * @param flag Prefixo esperado da flag (ex: "-r=").
 * @param flag_len Comprimento do prefixo da flag.
 * @return const char* Ponteiro para a substring do valor, ou nullptr se a flag não coincidir.
 */
static const char* extrair_valor(const char* arg, const char* flag, size_t flag_len)
{
    if (strncmp(arg, flag, flag_len) == 0) {
        return arg + flag_len;
    }
    return nullptr;
}

const Parameters analisar_args(int argc, char* argv[])
{
    Parameters params;
    params.class_file_path  = nullptr;
    params.output_file_path = nullptr;
    params.execute          = false;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        const char* v;

        if ((v = extrair_valor(arg, "-r=", 3)) != nullptr) {
            params.execute         = false;
            params.class_file_path = const_cast<char*>(v);
        }
        else if ((v = extrair_valor(arg, "-o=", 3)) != nullptr) {
            params.output_file_path = const_cast<char*>(v);
        }
        else if ((v = extrair_valor(arg, "-e=", 3)) != nullptr) {
            params.execute         = true;
            params.class_file_path = const_cast<char*>(v);
        }
    }
    return params;
}

bool validar_parametros(const Parameters* params)
{
    if (!params->class_file_path
        || (!params->execute && !params->output_file_path)) {
        cerr << "Uso:\n"
             << "  bin -r=caminho/Classe.class -o=saida.txt\n"
             << "  bin -e=caminho/Classe.class\n";
        return false;
    }
    return true;
}
