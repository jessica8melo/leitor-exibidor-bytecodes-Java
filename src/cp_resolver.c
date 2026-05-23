#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/cp_resolver.h"

char *get_utf8(ClassFile *cf, uint16_t index)
{
    cp_info *cp = cf->constant_pool;

    if (cp[index].tag != CONSTANT_Utf8) {
        return NULL;
    }

    uint16_t len = cp[index].info.utf8_info.length;

    char *str = malloc(len + 1);

    memcpy(
        str,
        cp[index].info.utf8_info.bytes,
        len
    );

    str[len] = '\0';

    return str;
}