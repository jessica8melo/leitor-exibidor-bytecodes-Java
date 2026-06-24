#ifndef OBJETO_HPP
#define OBJETO_HPP

#include "tipos_runtime.hpp"

class Objeto {
public:
    virtual ~Objeto() {}

    virtual TipoObjeto tipo_objeto() = 0;
};

#endif 
