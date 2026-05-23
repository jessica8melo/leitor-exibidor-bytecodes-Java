// tipos e funções do pool
#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H

#include <stdint.h>
#include <stdlib.h>

/*
 * ConstantPoolEntry e cp_get_utf8 ficam aqui.
 * fields.h, methods.h e attributes.h incluem este header.
 * Quando o M1 entregar o constant_pool.c real, só este arquivo muda.
 */

typedef struct {
    uint8_t  tag;
    void    *data;
} ConstantPoolEntry;

/* Retorna a string UTF-8 do índice `index` no pool, ou "?" se inválido */
const char *cp_get_utf8(const ConstantPoolEntry *cp, uint16_t index);

#endif /* CONSTANT_POOL_H */