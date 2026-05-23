#include "classfile.h"
#include "constant_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

u1 read_u1(FILE *fp);
u2 read_u2(FILE *fp);
u4 read_u4(FILE *fp);

static attribute_info read_attribute(FILE *fp)
{
    attribute_info attr;
    attr.attribute_name_index = read_u2(fp);
    attr.attribute_length     = read_u4(fp);
    if (attr.attribute_length > 0) {
        attr.data = malloc(attr.attribute_length);
        if (!attr.data) { fprintf(stderr, "[fields] sem memória\n"); exit(1); }
        fread(attr.data, 1, attr.attribute_length, fp);
    } else {
        attr.data = NULL;
    }
    return attr;
}

int read_fields(ClassFile *cf, FILE *fp)
{
    cf->fields_count = read_u2(fp);
    if (cf->fields_count == 0) { cf->fields = NULL; return 0; }

    cf->fields = calloc(cf->fields_count, sizeof(field_info));
    if (!cf->fields) return -1;

    for (u2 i = 0; i < cf->fields_count; i++) {
        field_info *f = &cf->fields[i];
        f->access_flags     = read_u2(fp);
        f->name_index       = read_u2(fp);
        f->descriptor_index = read_u2(fp);
        f->attributes_count = read_u2(fp);

        if (f->attributes_count > 0) {
            f->attributes = calloc(f->attributes_count, sizeof(attribute_info));
            if (!f->attributes) return -1;
            for (u2 j = 0; j < f->attributes_count; j++)
                f->attributes[j] = read_attribute(fp);
        } else {
            f->attributes = NULL;
        }
    }
    return 0;
}

void free_fields(ClassFile *cf)
{
    if (!cf->fields) return;
    for (u2 i = 0; i < cf->fields_count; i++) {
        field_info *f = &cf->fields[i];
        for (u2 j = 0; j < f->attributes_count; j++)
            free(f->attributes[j].data);
        free(f->attributes);
    }
    free(cf->fields);
    cf->fields = NULL;
}

static void field_flags_to_string(u2 flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { u2 mask; const char *name; } table[] = {
        { 0x0001, "public"    },
        { 0x0002, "private"   },
        { 0x0004, "protected" },
        { 0x0008, "static"    },
        { 0x0010, "final"     },
        { 0x0040, "volatile"  },
        { 0x0080, "transient" },
        { 0x1000, "synthetic" },
        { 0x4000, "enum"      },
    };
    for (size_t k = 0; k < sizeof(table)/sizeof(table[0]); k++) {
        if (flags & table[k].mask) {
            if (buf[0] != '\0') strncat(buf, " ", buf_size - strlen(buf) - 1);
            strncat(buf, table[k].name, buf_size - strlen(buf) - 1);
        }
    }
    if (buf[0] == '\0') strncpy(buf, "(none)", buf_size - 1);
}

void print_fields(ClassFile *cf, FILE *out)
{
    fprintf(out, "Fields (count: %d)\n{\n", cf->fields_count);

    if (cf->fields_count == 0) {
        fprintf(out, "\t (no fields)\n}\n\n");
        return;
    }

    for (u2 i = 0; i < cf->fields_count; i++) {
        field_info *f = &cf->fields[i];

        const char *name = "?", *descriptor = "?";
        if (f->name_index < cf->constant_pool_count &&
            cf->constant_pool[f->name_index].tag == CONSTANT_Utf8)
            name = (char *)cf->constant_pool[f->name_index].info.utf8_info.bytes;
        if (f->descriptor_index < cf->constant_pool_count &&
            cf->constant_pool[f->descriptor_index].tag == CONSTANT_Utf8)
            descriptor = (char *)cf->constant_pool[f->descriptor_index].info.utf8_info.bytes;

        char flags_str[64];
        field_flags_to_string(f->access_flags, flags_str, sizeof(flags_str));

        fprintf(out, "\t [%d] FieldInfo\n", i + 1);
        fprintf(out, "\t\t Name: \t\t\t\t ConstantPoolInfo #%d <%s>\n",
                f->name_index, name);
        fprintf(out, "\t\t Descriptor: \t\t\t ConstantPoolInfo #%d <%s>\n",
                f->descriptor_index, descriptor);
        fprintf(out, "\t\t Access Flags: \t\t\t 0x%04X [%s]\n",
                f->access_flags, flags_str);
        fprintf(out, "\t\t Attributes count: \t\t %d\n", f->attributes_count);

        for (u2 j = 0; j < f->attributes_count; j++) {
            attribute_info *a = &f->attributes[j];
            const char *aname = "?";
            if (a->attribute_name_index < cf->constant_pool_count &&
                cf->constant_pool[a->attribute_name_index].tag == CONSTANT_Utf8)
                aname = (char *)cf->constant_pool[a->attribute_name_index].info.utf8_info.bytes;
            fprintf(out, "\t\t [%d] %s\n", j + 1, aname);

            if (strcmp(aname, "ConstantValue") == 0 && a->attribute_length == 2) {
                u2 cv_idx = (u2)((a->data[0] << 8) | a->data[1]);
                const char *cv_val = "?";
                if (cv_idx < cf->constant_pool_count &&
                    cf->constant_pool[cv_idx].tag == CONSTANT_Utf8)
                    cv_val = (char *)cf->constant_pool[cv_idx].info.utf8_info.bytes;
                fprintf(out, "\t\t\t Constant Value: \t\t ConstantPoolInfo #%d <%s>\n",
                        cv_idx, cv_val);
            }
        }

        if (i + 1 < cf->fields_count) fprintf(out, "\n");
    }
    fprintf(out, "}\n\n");
}