#ifndef AREA_METODOS_HPP
#define AREA_METODOS_HPP

#include "classe_estatica.hpp"
#include <map>
#include <string>

class AreaMetodos {
public:

    static AreaMetodos& instancia()
    {
        static AreaMetodos unica;
        return unica;
    }

    ClasseEstatica* carregar_classe(const std::string& nome);

    ClasseEstatica* obter_classe(const std::string& nome);

    void registrar_classe(ClasseEstatica* c);

    std::string class_path = "";

    AreaMetodos(const AreaMetodos&)            = delete;
    AreaMetodos& operator=(const AreaMetodos&) = delete;

private:
    AreaMetodos() = default;
    ~AreaMetodos() = default;

    bool inserir_classe(ClasseEstatica* classe_runtime);

    std::map<std::string, ClasseEstatica*> classes;
};

#endif 
