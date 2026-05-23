#include "classfile.h"
#include "constant_pool.h"
#include "cp_resolver.h"
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
        if (!attr.data) { fprintf(stderr, "[methods] sem memória\n"); exit(1); }
        fread(attr.data, 1, attr.attribute_length, fp);
    } else {
        attr.data = NULL;
    }
    return attr;
}

int read_methods(ClassFile *cf, FILE *fp)
{
    cf->methods_count = read_u2(fp);
    if (cf->methods_count == 0) { cf->methods = NULL; return 0; }

    cf->methods = calloc(cf->methods_count, sizeof(method_info));
    if (!cf->methods) return -1;

    for (u2 i = 0; i < cf->methods_count; i++) {
        method_info *m = &cf->methods[i];
        m->access_flags     = read_u2(fp);
        m->name_index       = read_u2(fp);
        m->descriptor_index = read_u2(fp);
        m->attributes_count = read_u2(fp);

        if (m->attributes_count > 0) {
            m->attributes = calloc(m->attributes_count, sizeof(attribute_info));
            if (!m->attributes) return -1;
            for (u2 j = 0; j < m->attributes_count; j++)
                m->attributes[j] = read_attribute(fp);
        } else {
            m->attributes = NULL;
        }
    }
    return 0;
}

void free_methods(ClassFile *cf)
{
    if (!cf->methods) return;
    for (u2 i = 0; i < cf->methods_count; i++) {
        method_info *m = &cf->methods[i];
        for (u2 j = 0; j < m->attributes_count; j++)
            free(m->attributes[j].data);
        free(m->attributes);
    }
    free(cf->methods);
    cf->methods = NULL;
}

static void method_flags_to_string(u2 flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { u2 mask; const char *name; } table[] = {
        { 0x0001, "public"       },
        { 0x0002, "private"      },
        { 0x0004, "protected"    },
        { 0x0008, "static"       },
        { 0x0010, "final"        },
        { 0x0020, "synchronized" },
        { 0x0040, "bridge"       },
        { 0x0080, "varargs"      },
        { 0x0100, "native"       },
        { 0x0400, "abstract"     },
        { 0x0800, "strictfp"     },
        { 0x1000, "synthetic"    },
    };
    for (size_t k = 0; k < sizeof(table)/sizeof(table[0]); k++) {
        if (flags & table[k].mask) {
            if (buf[0] != '\0') strncat(buf, " ", buf_size - strlen(buf) - 1);
            strncat(buf, table[k].name, buf_size - strlen(buf) - 1);
        }
    }
    if (buf[0] == '\0') strncpy(buf, "(none)", buf_size - 1);
}

static void print_code_attribute(ClassFile *cf, attribute_info *attr, FILE *out)
{
    u1 *d = attr->data;
    u2 max_stack  = (u2)((d[0] << 8) | d[1]);
    u2 max_locals = (u2)((d[2] << 8) | d[3]);
    u4 code_len   = ((u4)d[4] << 24) | ((u4)d[5] << 16) | ((u4)d[6] << 8) | d[7];
    u1 *code      = d + 8;

    fprintf(out, "\t\t\t Max Stack: \t\t\t %d\n", max_stack);
    fprintf(out, "\t\t\t Max Locals: \t\t\t %d\n", max_locals);
    fprintf(out, "\t\t\t Code Length: \t\t\t %u\n", code_len);

    fprintf(out, "\t\t\t Bytecodes (hex):\n\t\t\t\t ");
    for (u4 b = 0; b < code_len; b++) {
        fprintf(out, "%02X ", code[b]);
        if ((b + 1) % 16 == 0 && b + 1 < code_len)
            fprintf(out, "\n\t\t\t\t ");
    }
    fprintf(out, "\n");

    u4 pos = 8 + code_len;
    u2 ex_len = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
    fprintf(out, "\t\t\t Exception Table Length: \t %d\n", ex_len);
    for (u2 k = 0; k < ex_len; k++) {
        u2 start_pc   = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
        u2 end_pc     = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
        u2 handler_pc = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
        u2 catch_type = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
        const char *cname = catch_type == 0 ? "any" : "?";
        if (catch_type != 0 && catch_type < cf->constant_pool_count &&
            cf->constant_pool[catch_type].tag == CONSTANT_Class) {
            u2 ni = cf->constant_pool[catch_type].info.class_info.name_index;
            if (ni < cf->constant_pool_count &&
                cf->constant_pool[ni].tag == CONSTANT_Utf8)
                cname = (char *)cf->constant_pool[ni].info.utf8_info.bytes;
        }
        fprintf(out, "\t\t\t\t [%d] start=%d end=%d handler=%d catch=<%s>\n",
                k+1, start_pc, end_pc, handler_pc, cname);
    }

    u2 sub_count = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
    fprintf(out, "\t\t\t Attributes count: \t\t %d\n", sub_count);
    for (u2 k = 0; k < sub_count; k++) {
        u2 an = (u2)((d[pos] << 8) | d[pos+1]); pos += 2;
        u4 al = ((u4)d[pos] << 24) | ((u4)d[pos+1] << 16) |
                ((u4)d[pos+2] << 8) | d[pos+3]; pos += 4;
        const char *aname = "?";
        if (an < cf->constant_pool_count &&
            cf->constant_pool[an].tag == CONSTANT_Utf8)
            aname = (char *)cf->constant_pool[an].info.utf8_info.bytes;
        fprintf(out, "\t\t\t\t [%d] %s (%u bytes)\n", k+1, aname, al);
        pos += al;
    }
}

void print_methods(ClassFile *cf, FILE *out)
{
    fprintf(out, "Methods (count: %d)\n{\n", cf->methods_count);

    if (cf->methods_count == 0) {
        fprintf(out, "\t (no methods)\n}\n\n");
        return;
    }

    for (u2 i = 0; i < cf->methods_count; i++) {
        method_info *m = &cf->methods[i];

        const char *name = "?", *descriptor = "?";
        if (m->name_index < cf->constant_pool_count &&
            cf->constant_pool[m->name_index].tag == CONSTANT_Utf8)
            name = (char *)cf->constant_pool[m->name_index].info.utf8_info.bytes;
        if (m->descriptor_index < cf->constant_pool_count &&
            cf->constant_pool[m->descriptor_index].tag == CONSTANT_Utf8)
            descriptor = (char *)cf->constant_pool[m->descriptor_index].info.utf8_info.bytes;

        char flags_str[128];
        method_flags_to_string(m->access_flags, flags_str, sizeof(flags_str));

        fprintf(out, "\t [%d] MethodInfo\n", i + 1);
        fprintf(out, "\t\t Name: \t\t\t\t ConstantPoolInfo #%d <%s>\n",
                m->name_index, name);
        fprintf(out, "\t\t Descriptor: \t\t\t ConstantPoolInfo #%d <%s>\n",
                m->descriptor_index, descriptor);
        fprintf(out, "\t\t Access Flags: \t\t\t 0x%04X [%s]\n",
                m->access_flags, flags_str);
        fprintf(out, "\t\t Attributes count: \t\t %d\n", m->attributes_count);

        for (u2 j = 0; j < m->attributes_count; j++) {
            attribute_info *a = &m->attributes[j];
            const char *aname = "?";
            if (a->attribute_name_index < cf->constant_pool_count &&
                cf->constant_pool[a->attribute_name_index].tag == CONSTANT_Utf8)
                aname = (char *)cf->constant_pool[a->attribute_name_index].info.utf8_info.bytes;

            fprintf(out, "\t\t [%d] %s\n", j + 1, aname);

            if (strcmp(aname, "Code") == 0 && a->data != NULL)
                print_code_attribute(cf, a, out);
            else if (strcmp(aname, "Exceptions") == 0 && a->data != NULL) {
                u2 n = (u2)((a->data[0] << 8) | a->data[1]);
                fprintf(out, "\t\t\t Number of Exceptions: \t\t %d\n", n);
                for (u2 k = 0; k < n; k++) {
                    u2 idx = (u2)((a->data[2 + k*2] << 8) | a->data[3 + k*2]);
                    const char *ex = "?";
                    if (idx < cf->constant_pool_count &&
                        cf->constant_pool[idx].tag == CONSTANT_Class) {
                        u2 ni = cf->constant_pool[idx].info.class_info.name_index;
                        if (ni < cf->constant_pool_count &&
                            cf->constant_pool[ni].tag == CONSTANT_Utf8)
                            ex = (char *)cf->constant_pool[ni].info.utf8_info.bytes;
                    }
                    fprintf(out, "\t\t\t\t [%d] ConstantPoolInfo #%d <%s>\n",
                            k+1, idx, ex);
                }
            }
        }

        if (i + 1 < cf->methods_count) fprintf(out, "\n");
    }
    fprintf(out, "}\n\n");
}