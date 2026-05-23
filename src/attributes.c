#include "classfile.h"
#include "constant_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

u1 read_u1(FILE *fp);
u2 read_u2(FILE *fp);
u4 read_u4(FILE *fp);


int read_class_attributes(ClassFile *cf, FILE *fp)
{
    cf->attributes_count = read_u2(fp);
    if (cf->attributes_count == 0) { cf->attributes = NULL; return 0; }

    cf->attributes = calloc(cf->attributes_count, sizeof(attribute_info));
    if (!cf->attributes) return -1;

    for (u2 i = 0; i < cf->attributes_count; i++) {
        attribute_info *a = &cf->attributes[i];
        a->attribute_name_index = read_u2(fp);
        a->attribute_length     = read_u4(fp);
        if (a->attribute_length > 0) {
            a->data = malloc(a->attribute_length);
            if (!a->data) { fprintf(stderr, "[attributes] sem memória\n"); exit(1); }
            fread(a->data, 1, a->attribute_length, fp);
        } else {
            a->data = NULL;
        }
    }
    return 0;
}


void free_class_attributes(ClassFile *cf)
{
    if (!cf->attributes) return;
    for (u2 i = 0; i < cf->attributes_count; i++)
        free(cf->attributes[i].data);
    free(cf->attributes);
    cf->attributes = NULL;
}

void print_class_attributes(ClassFile *cf, FILE *out)
{
    fprintf(out, "Class Attributes (count: %d)\n{\n", cf->attributes_count);

    if (cf->attributes_count == 0) {
        fprintf(out, "\t (no attributes)\n}\n\n");
        return;
    }

    for (u2 i = 0; i < cf->attributes_count; i++) {
        attribute_info *a = &cf->attributes[i];
        const char *aname = "?";
        if (a->attribute_name_index < cf->constant_pool_count &&
            cf->constant_pool[a->attribute_name_index].tag == CONSTANT_Utf8)
            aname = (char *)cf->constant_pool[a->attribute_name_index].info.utf8_info.bytes;

        fprintf(out, "\t [%d] %s\n", i + 1, aname);
        fprintf(out, "\t\t Attribute Name: \t\t ConstantPoolInfo #%d <%s>\n",
                a->attribute_name_index, aname);
        fprintf(out, "\t\t Attribute Length: \t\t %u\n", a->attribute_length);

        if (strcmp(aname, "SourceFile") == 0 && a->attribute_length == 2) {
            u2 idx = (u2)((a->data[0] << 8) | a->data[1]);
            const char *fname = "?";
            if (idx < cf->constant_pool_count &&
                cf->constant_pool[idx].tag == CONSTANT_Utf8)
                fname = (char *)cf->constant_pool[idx].info.utf8_info.bytes;
            fprintf(out, "\t\t SourceFile: \t\t\t ConstantPoolInfo #%d <%s>\n",
                    idx, fname);
        } else if (strcmp(aname, "Signature") == 0 && a->attribute_length == 2) {
            u2 idx = (u2)((a->data[0] << 8) | a->data[1]);
            const char *sig = "?";
            if (idx < cf->constant_pool_count &&
                cf->constant_pool[idx].tag == CONSTANT_Utf8)
                sig = (char *)cf->constant_pool[idx].info.utf8_info.bytes;
            fprintf(out, "\t\t Signature: \t\t\t ConstantPoolInfo #%d <%s>\n",
                    idx, sig);
        } else if (a->attribute_length > 0) {
            fprintf(out, "\t\t (raw bytes, %u bytes)\n", a->attribute_length);
        }

        if (i + 1 < cf->attributes_count) fprintf(out, "\n");
    }
    fprintf(out, "}\n\n");
}