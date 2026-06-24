#ifndef CLASSE_INSTANCIA_HPP
#define CLASSE_INSTANCIA_HPP

#include "classe_estatica.hpp"
#include "objeto.hpp"
#include "tipos_runtime.hpp"
#include <map>
#include <string>

class ClasseInstancia : public Objeto {
public:

    ClasseInstancia(ClasseEstatica* classe_runtime);

    ~ClasseInstancia();

    TipoObjeto tipo_objeto();

    ClasseEstatica* get_classe_runtime();

    void insert_value_into_field(Valor valor, const std::string& nome_campo);

    Valor get_value_from_field(const std::string& nome_campo);

    bool field_exists(const std::string& nome_campo);

private:

    ClasseEstatica* classe_runtime;

    std::map<std::string, Valor> campos_instancia;
};

#endif 
