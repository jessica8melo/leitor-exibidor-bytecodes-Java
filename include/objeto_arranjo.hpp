#ifndef OBJETO_ARRANJO_HPP
#define OBJETO_ARRANJO_HPP

#include "objeto.hpp"
#include <cstdint>
#include <vector>

class Arranjo : public Objeto {
public:

    Arranjo(TipoValor tipo);
    ~Arranjo();

    TipoObjeto tipo_objeto();

    TipoValor array_content_type();

    void push_value(Valor value);

    Valor remove_at(uint32_t index);

    Valor remove_last();

    Valor remove_first();

    uint32_t get_size();

    Valor get_value(uint32_t index);

    void change_value(uint32_t index, Valor value);

private:
    TipoValor array_type;                 
    std::vector<Valor> array_elements;    
};

#endif 
