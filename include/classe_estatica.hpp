#ifndef CLASSE_ESTATICA_HPP
#define CLASSE_ESTATICA_HPP

#include "arquivo_classe.hpp"
#include "tipos_runtime.hpp"
#include <map>
#include <string>

class ClasseEstatica {
public:

    ClasseEstatica(ArquivoClasse* arquivo_classe);

    ArquivoClasse* get_arquivo_classe();

    void insert_value(const Valor& valor, const std::string& nome_campo);

    Valor get_value(const std::string& nome_campo);

    bool check_field(const std::string& nome_campo);

private:

    ArquivoClasse* arquivo_classe;

    std::map<std::string, Valor> campos_estaticos;
};

#endif 
