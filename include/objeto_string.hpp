#ifndef OBJETO_STRING_HPP
#define OBJETO_STRING_HPP

#include "objeto.hpp"
#include <string>

class ObjetoString : public Objeto {
public:

    ObjetoString(std::string s = "");
    ~ObjetoString();

    TipoObjeto tipo_objeto();

    std::string get_str();

    void set_str(std::string s);

private:

    std::string str_interna;
};

#endif 
