#include "args.hpp"
#include "leitor_classe.hpp"
#include "exibidor_classe.hpp"
#include "area_metodos.hpp"
#include "classe_estatica.hpp"
#include "executor.hpp"
#include <cstdio>
#include <iostream>
#include <string>

static std::string diretorio_de(const char* caminho)
{
    std::string s(caminho);
    for (char& c : s) {
        if (c == '\\') c = '/';
    }
    const size_t pos = s.rfind('/');
    if (pos == std::string::npos) return "";
    return s.substr(0, pos + 1);
}

int main(int argc, char* argv[])
{
    std::cout << std::fixed;

    const Parameters params = analisar_args(argc, argv);
    if (!validar_parametros(&params)) {
        return -1;
    }

    FILE* fp = std::fopen(params.class_file_path, "rb");
    if (fp == nullptr) {
        std::cerr << "Could not find file at \"" << params.class_file_path << "\"\n";
        return -1;
    }

    FILE* saida = nullptr;
    if (!params.execute) {
        saida = std::fopen(params.output_file_path, "w+");
        if (saida == nullptr) {
            std::cerr << "Could not write to output file at \""
                      << params.output_file_path << "\"\n";
            std::fclose(fp);
            return -1;
        }
    }

    LeitorClasse& leitor = LeitorClasse::instancia();
    ArquivoClasse* arq   = leitor.ler_arquivo(fp);
    std::fclose(fp);

    if (params.execute) {
        AreaMetodos& area = AreaMetodos::instancia();
        area.class_path   = diretorio_de(params.class_file_path);

        ClasseEstatica* c = new ClasseEstatica(arq);
        area.registrar_classe(c);

        Executor::instancia().executar_metodos(c);
    } else {
        escrever_arquivo(arq, saida);
        std::fclose(saida);
    }

    return 0;
}
