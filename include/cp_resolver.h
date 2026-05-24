#ifndef CP_RESOLVER_H
#define CP_RESOLVER_H

#include "classfile.h"

char *get_utf8(
    ClassFile *cf,
    uint16_t index
);

char *resolve_cp_for_bytecode(
    ClassFile *cf,
    u2 index
);

#endif