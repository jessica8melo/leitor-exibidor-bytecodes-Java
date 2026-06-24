#ifndef ARQUIVO_CLASSE_HPP
#define ARQUIVO_CLASSE_HPP

#include "tipos_basicos.hpp"

class ArquivoClasse {
public:
    ArquivoClasse() = default;

    u4 magic;
    u2 minor_version;
    u2 major_version;

    u2 constant_pool_count;
    ConstantPoolInfo* constant_pool;

    u2 access_flags;
    u2 this_class;
    u2 super_class;

    u2 interfaces_count;
    u2* interfaces;

    u2 fields_count;
    FieldInfo* fields;

    u2 methods_count;
    MethodInfo* methods;

    u2 attributes_count;
    AttributeInfo* attributes;
};

#endif
