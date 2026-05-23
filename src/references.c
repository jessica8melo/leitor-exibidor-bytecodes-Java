#include "classfile.h"
#include "constant_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int read_fields(ClassFile *cf, FILE *fp);
int read_methods(ClassFile *cf, FILE *fp);
int read_class_attributes(ClassFile *cf, FILE *fp);


u1 read_u1(FILE *fp) {
    u1 res;
    if (fread(&res, 1, 1, fp) != 1) return 0;
    return res;
}

u2 read_u2(FILE *fp) {
    return (u2)((read_u1(fp) << 8) | read_u1(fp));
}

u4 read_u4(FILE *fp) {
    return ((u4)read_u1(fp) << 24) | ((u4)read_u1(fp) << 16) |
           ((u4)read_u1(fp) <<  8) |  (u4)read_u1(fp);
}


ClassFile *read_class_file(FILE *fp)
{
    ClassFile *cf = calloc(1, sizeof(ClassFile));
    if (!cf) return NULL;

    cf->magic = read_u4(fp);
    if (cf->magic != 0xCAFEBABE) {
        fprintf(stderr, "Erro: magic number inválido (0x%08X)\n", cf->magic);
        free(cf);
        return NULL;
    }

    cf->minor_version = read_u2(fp);
    cf->major_version = read_u2(fp);

    if (read_constant_pool(cf, fp) != 0) { free(cf); return NULL; }

    cf->access_flags = read_u2(fp);
    cf->this_class   = read_u2(fp);
    cf->super_class  = read_u2(fp);

    cf->interfaces_count = read_u2(fp);
    if (cf->interfaces_count > 0) {
        cf->interfaces = calloc(cf->interfaces_count, sizeof(u2));
        for (u2 i = 0; i < cf->interfaces_count; i++)
            cf->interfaces[i] = read_u2(fp);
    }

    if (read_fields(cf, fp)           != 0) { free_class_file(cf); return NULL; }
    if (read_methods(cf, fp)          != 0) { free_class_file(cf); return NULL; }
    if (read_class_attributes(cf, fp) != 0) { free_class_file(cf); return NULL; }

    return cf;
}

void free_fields(ClassFile *cf);
void free_methods(ClassFile *cf);
void free_class_attributes(ClassFile *cf);

void free_class_file(ClassFile *cf)
{
    if (!cf) return;
    free_constant_pool(cf);
    free_fields(cf);
    free_methods(cf);
    free_class_attributes(cf);
    free(cf->interfaces);
    free(cf);
}