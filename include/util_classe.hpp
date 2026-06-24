#ifndef UTIL_CLASSE_HPP
#define UTIL_CLASSE_HPP

#include "tipos_basicos.hpp"
#include "arquivo_classe.hpp"
#include <cstdio>
#include <sstream>
#include <string>

class UtilClasse {
public:

    static double versao_legivel(ArquivoClasse* arq);
    static double versao_para_msg_erro(ArquivoClasse* arq);

    static bool utf8_igual_a(ConstUtf8Info entry, const char* alvo);

    static void indentar_arquivo(FILE* out, uint8_t n);
    static void indentar_tela(uint8_t n);

    static const char* ss_para_cstr(const std::stringstream& ss);

    static bool conferir_nome_classe(std::string caminho_arq, std::string nome_classe);
};

#endif
