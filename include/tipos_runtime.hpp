#ifndef TIPOS_RUNTIME_HPP
#define TIPOS_RUNTIME_HPP

#include <stdint.h>

class Objeto;

enum TipoObjeto {
    INSTANCIA_CLASSE,  
    INSTANCIA_STRING,  
    ARRANJO            
};

enum TipoValor {
    BOOLEANO,
    BYTE,
    CHAR,
    SHORT,
    INT,
    FLOAT,
    LONG,
    DOUBLE,
    ENDERECO_RETORNO,
    REFERENCIA,
    PADDING
};

struct Valor {
    TipoValor tipo_print;  
    TipoValor tipo;        
    union {
        bool     valor_booleano;
        int8_t   valor_byte;
        uint8_t  valor_char;
        int16_t  valor_short;
        int32_t  valor_int;
        float    valor_float;
        int64_t  valor_long;
        double   valor_double;
        uint32_t endereco_retorno;
        Objeto*  objeto;
    } dados;
};

inline Valor faz_valor_booleano(bool v) {
    Valor x; x.tipo = BOOLEANO; x.tipo_print = BOOLEANO; x.dados.valor_booleano = v; return x;
}
inline Valor faz_valor_byte(int8_t v) {
    Valor x; x.tipo = BYTE; x.tipo_print = BYTE; x.dados.valor_byte = v; return x;
}
inline Valor faz_valor_char(uint8_t v) {
    Valor x; x.tipo = CHAR; x.tipo_print = CHAR; x.dados.valor_char = v; return x;
}
inline Valor faz_valor_short(int16_t v) {
    Valor x; x.tipo = SHORT; x.tipo_print = SHORT; x.dados.valor_short = v; return x;
}
inline Valor faz_valor_int(int32_t v) {
    Valor x; x.tipo = INT; x.tipo_print = INT; x.dados.valor_int = v; return x;
}
inline Valor faz_valor_float(float v) {
    Valor x; x.tipo = FLOAT; x.tipo_print = FLOAT; x.dados.valor_float = v; return x;
}
inline Valor faz_valor_long(int64_t v) {
    Valor x; x.tipo = LONG; x.tipo_print = LONG; x.dados.valor_long = v; return x;
}
inline Valor faz_valor_double(double v) {
    Valor x; x.tipo = DOUBLE; x.tipo_print = DOUBLE; x.dados.valor_double = v; return x;
}
inline Valor faz_valor_endereco_retorno(uint32_t v) {
    Valor x; x.tipo = ENDERECO_RETORNO; x.tipo_print = ENDERECO_RETORNO; x.dados.endereco_retorno = v; return x;
}
inline Valor faz_valor_referencia(Objeto* v) {
    Valor x; x.tipo = REFERENCIA; x.tipo_print = REFERENCIA; x.dados.objeto = v; return x;
}
inline Valor faz_valor_padding() {
    Valor x; x.tipo = PADDING; x.tipo_print = PADDING; x.dados.valor_long = 0; return x;
}

#endif 
