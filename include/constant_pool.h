#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H

#include "classfile.h"
#include "cp_resolver.h"
#include <stdio.h>

int read_constant_pool(ClassFile *cf, FILE *fp);
void free_constant_pool(ClassFile *cf);

#endif /* CONSTANT_POOL_H */
