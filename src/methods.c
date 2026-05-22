#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "methods.h"

static uint8_t read_u1(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 1 > data_len) {
        fprintf(stderr, "[methods] Erro: leitura fora dos limites (u1 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }
    return data[(*offset)++];
}

static uint16_t read_u2(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 2 > data_len) {
        fprintf(stderr, "[methods] Erro: leitura fora dos limites (u2 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }
    uint16_t val = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
    *offset += 2;
    return val;
}

static uint32_t read_u4(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 4 > data_len) {
        fprintf(stderr, "[methods] Erro: leitura fora dos limites (u4 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }
    uint32_t val = ((uint32_t)data[*offset]     << 24) |
                   ((uint32_t)data[*offset + 1] << 16) |
                   ((uint32_t)data[*offset + 2] <<  8) |
                   ((uint32_t)data[*offset + 3]);
    *offset += 4;
    return val;
}

static MethodAttributeTag resolve_method_attr_tag(const char *name)
{
    if (!name) return MATTR_UNKNOWN;
    if (strcmp(name, "Code")                                    == 0) return MATTR_CODE;
    if (strcmp(name, "Exceptions")                              == 0) return MATTR_EXCEPTIONS;
    if (strcmp(name, "Signature")                               == 0) return MATTR_SIGNATURE;
    if (strcmp(name, "Deprecated")                              == 0) return MATTR_DEPRECATED;
    if (strcmp(name, "Synthetic")                               == 0) return MATTR_SYNTHETIC;
    if (strcmp(name, "AnnotationDefault")                       == 0) return MATTR_ANNOTATION_DEFAULT;
    if (strcmp(name, "RuntimeVisibleAnnotations")               == 0) return MATTR_RUNTIME_VISIBLE_ANNOTATIONS;
    if (strcmp(name, "RuntimeInvisibleAnnotations")             == 0) return MATTR_RUNTIME_INVISIBLE_ANNOTATIONS;
    if (strcmp(name, "RuntimeVisibleParameterAnnotations")      == 0) return MATTR_RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS;
    if (strcmp(name, "RuntimeInvisibleParameterAnnotations")    == 0) return MATTR_RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS;
    return MATTR_UNKNOWN;
}

static void parse_code_attribute(const uint8_t *data, uint32_t data_len,
                                 uint32_t *offset, CodeAttribute *code)
{
    code->max_stack  = read_u2(data, data_len, offset);
    code->max_locals = read_u2(data, data_len, offset);
    code->code_length = read_u4(data, data_len, offset);

    if (code->code_length > 0) {
        code->code_bytes = malloc(code->code_length);
        if (!code->code_bytes) {
            fprintf(stderr, "[methods] Erro: sem memória para bytecodes\n");
            exit(EXIT_FAILURE);
        }
        memcpy(code->code_bytes, data + *offset, code->code_length);
        *offset += code->code_length;
    } else {
        code->code_bytes = NULL;
    }

    code->exception_table_length = read_u2(data, data_len, offset);
    if (code->exception_table_length > 0) {
        code->exception_table = calloc(code->exception_table_length,
                                       sizeof(*code->exception_table));
        if (!code->exception_table) {
            fprintf(stderr, "[methods] Erro: sem memória para exception_table\n");
            exit(EXIT_FAILURE);
        }
        for (uint16_t k = 0; k < code->exception_table_length; k++) {
            code->exception_table[k].start_pc   = read_u2(data, data_len, offset);
            code->exception_table[k].end_pc     = read_u2(data, data_len, offset);
            code->exception_table[k].handler_pc = read_u2(data, data_len, offset);
            code->exception_table[k].catch_type = read_u2(data, data_len, offset);
        }
    } else {
        code->exception_table = NULL;
    }

    code->attributes_count = read_u2(data, data_len, offset);
    if (code->attributes_count > 0) {
        code->attributes = calloc(code->attributes_count, sizeof(*code->attributes));
        if (!code->attributes) {
            fprintf(stderr, "[methods] Erro: sem memória para sub-atributos do Code\n");
            exit(EXIT_FAILURE);
        }
        for (uint16_t k = 0; k < code->attributes_count; k++) {
            code->attributes[k].attribute_name_index = read_u2(data, data_len, offset);
            code->attributes[k].attribute_length     = read_u4(data, data_len, offset);
            uint32_t al = code->attributes[k].attribute_length;
            if (al > 0) {
                code->attributes[k].raw_bytes = malloc(al);
                if (!code->attributes[k].raw_bytes) {
                    fprintf(stderr, "[methods] Erro: sem memória para sub-atributo\n");
                    exit(EXIT_FAILURE);
                }
                memcpy(code->attributes[k].raw_bytes, data + *offset, al);
            } else {
                code->attributes[k].raw_bytes = NULL;
            }
            *offset += al;
        }
    } else {
        code->attributes = NULL;
    }
}

static void parse_method_attribute(const uint8_t *data, uint32_t data_len,
                                   uint32_t *offset, MethodAttributeInfo *attr,
                                   const ConstantPoolEntry *cp)
{
    attr->attribute_name_index = read_u2(data, data_len, offset);
    attr->attribute_length     = read_u4(data, data_len, offset);

    uint32_t end_offset = *offset + attr->attribute_length;

    const char *attr_name = cp_get_utf8(cp, attr->attribute_name_index);
    attr->tag = resolve_method_attr_tag(attr_name);

    if (attr->tag == MATTR_CODE) {
        parse_code_attribute(data, data_len, offset, &attr->info.code);

    } else if (attr->tag == MATTR_EXCEPTIONS) {
        attr->info.exceptions.number_of_exceptions = read_u2(data, data_len, offset);
        uint16_t n = attr->info.exceptions.number_of_exceptions;
        if (n > 0) {
            attr->info.exceptions.exception_index_table = calloc(n, sizeof(uint16_t));
            if (!attr->info.exceptions.exception_index_table) {
                fprintf(stderr, "[methods] Erro: sem memória para exceptions\n");
                exit(EXIT_FAILURE);
            }
            for (uint16_t k = 0; k < n; k++)
                attr->info.exceptions.exception_index_table[k] =
                    read_u2(data, data_len, offset);
        } else {
            attr->info.exceptions.exception_index_table = NULL;
        }

    } else if (attr->tag == MATTR_SIGNATURE) {
        attr->info.signature.signature_index = read_u2(data, data_len, offset);

    } else if (attr->tag == MATTR_DEPRECATED || attr->tag == MATTR_SYNTHETIC) {

    } else {
        attr->info.unknown.length = attr->attribute_length;
        if (attr->attribute_length > 0) {
            attr->info.unknown.data = malloc(attr->attribute_length);
            if (!attr->info.unknown.data) {
                fprintf(stderr, "[methods] Erro: sem memória para atributo desconhecido\n");
                exit(EXIT_FAILURE);
            }
            memcpy(attr->info.unknown.data, data + *offset, attr->attribute_length);
        } else {
            attr->info.unknown.data = NULL;
        }
        *offset += attr->attribute_length;
    }

    if (*offset != end_offset) {
        fprintf(stderr, "[methods] Aviso: offset inconsistente após atributo '%s' "
                        "(esperado %u, atual %u)\n",
                attr_name ? attr_name : "?", end_offset, *offset);
        *offset = end_offset;
    }
}

MethodInfo *parse_methods(const uint8_t *data, uint32_t data_len,
                          uint32_t *offset, uint16_t methods_count,
                          const ConstantPoolEntry *cp)
{
    if (methods_count == 0) return NULL;

    MethodInfo *methods = calloc(methods_count, sizeof(MethodInfo));
    if (!methods) {
        fprintf(stderr, "[methods] Erro: sem memória para methods\n");
        exit(EXIT_FAILURE);
    }

    for (uint16_t i = 0; i < methods_count; i++) {
        MethodInfo *m = &methods[i];
        m->access_flags     = read_u2(data, data_len, offset);
        m->name_index       = read_u2(data, data_len, offset);
        m->descriptor_index = read_u2(data, data_len, offset);
        m->attributes_count = read_u2(data, data_len, offset);

        if (m->attributes_count > 0) {
            m->attributes = calloc(m->attributes_count, sizeof(MethodAttributeInfo));
            if (!m->attributes) {
                fprintf(stderr, "[methods] Erro: sem memória para atributos do method %u\n", i);
                exit(EXIT_FAILURE);
            }
            for (uint16_t j = 0; j < m->attributes_count; j++)
                parse_method_attribute(data, data_len, offset, &m->attributes[j], cp);
        } else {
            m->attributes = NULL;
        }
    }

    return methods;
}

static void method_flags_to_string(uint16_t flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { uint16_t mask; const char *name; } table[] = {
        { MACC_PUBLIC,       "public"       },
        { MACC_PRIVATE,      "private"      },
        { MACC_PROTECTED,    "protected"    },
        { MACC_STATIC,       "static"       },
        { MACC_FINAL,        "final"        },
        { MACC_SYNCHRONIZED, "synchronized" },
        { MACC_BRIDGE,       "bridge"       },
        { MACC_VARARGS,      "varargs"      },
        { MACC_NATIVE,       "native"       },
        { MACC_ABSTRACT,     "abstract"     },
        { MACC_STRICT,       "strictfp"     },
        { MACC_SYNTHETIC,    "synthetic"    },
    };
    size_t n = sizeof(table) / sizeof(table[0]);
    for (size_t k = 0; k < n; k++) {
        if (flags & table[k].mask) {
            if (buf[0] != '\0')
                strncat(buf, " ", buf_size - strlen(buf) - 1);
            strncat(buf, table[k].name, buf_size - strlen(buf) - 1);
        }
    }
    if (buf[0] == '\0')
        strncpy(buf, "(none)", buf_size - 1);
}

static void print_code_attribute(const CodeAttribute *code,
                                 const ConstantPoolEntry *cp)
{
    printf("\t\t\t Max Stack: \t\t\t %u\n",  code->max_stack);
    printf("\t\t\t Max Locals: \t\t\t %u\n", code->max_locals);
    printf("\t\t\t Code Length: \t\t\t %u\n", code->code_length);

    printf("\t\t\t Bytecodes (hex):\n\t\t\t\t ");
    for (uint32_t b = 0; b < code->code_length; b++) {
        printf("%02X ", code->code_bytes[b]);
        if ((b + 1) % 16 == 0 && b + 1 < code->code_length)
            printf("\n\t\t\t\t ");
    }
    printf("\n");

    printf("\t\t\t Exception Table Length: \t %u\n", code->exception_table_length);
    for (uint16_t k = 0; k < code->exception_table_length; k++) {
        const char *catch_name = (code->exception_table[k].catch_type != 0)
            ? cp_get_utf8(cp, code->exception_table[k].catch_type)
            : "any";
        printf("\t\t\t\t [%u] start=%u end=%u handler=%u catch=<%s>\n",
               k + 1,
               code->exception_table[k].start_pc,
               code->exception_table[k].end_pc,
               code->exception_table[k].handler_pc,
               catch_name ? catch_name : "?");
    }

    printf("\t\t\t Attributes count: \t\t %u\n", code->attributes_count);
    for (uint16_t k = 0; k < code->attributes_count; k++) {
        const char *sub_name = cp_get_utf8(cp, code->attributes[k].attribute_name_index);
        printf("\t\t\t\t [%u] %s (%u bytes)\n", k + 1,
               sub_name ? sub_name : "?",
               code->attributes[k].attribute_length);
    }
}

static void print_method_attributes(const MethodAttributeInfo *attrs,
                                    uint16_t count,
                                    const ConstantPoolEntry *cp)
{
    for (uint16_t j = 0; j < count; j++) {
        const MethodAttributeInfo *a = &attrs[j];
        const char *attr_name = cp_get_utf8(cp, a->attribute_name_index);
        printf("\t\t [%u] %s\n", j + 1, attr_name ? attr_name : "?");

        if (a->tag == MATTR_CODE) {
            print_code_attribute(&a->info.code, cp);

        } else if (a->tag == MATTR_EXCEPTIONS) {
            uint16_t n = a->info.exceptions.number_of_exceptions;
            printf("\t\t\t Number of Exceptions: \t\t %u\n", n);
            for (uint16_t k = 0; k < n; k++) {
                uint16_t idx = a->info.exceptions.exception_index_table[k];
                const char *ex = cp_get_utf8(cp, idx);
                printf("\t\t\t\t [%u] ConstantPoolInfo #%u <%s>\n",
                       k + 1, idx, ex ? ex : "?");
            }

        } else if (a->tag == MATTR_SIGNATURE) {
            uint16_t idx = a->info.signature.signature_index;
            const char *sig = cp_get_utf8(cp, idx);
            printf("\t\t\t Signature: \t\t\t ConstantPoolInfo #%u <%s>\n",
                   idx, sig ? sig : "?");

        } else if (a->tag == MATTR_DEPRECATED) {

        } else if (a->tag == MATTR_SYNTHETIC) {

        } else {
            printf("\t\t\t (raw bytes, %u bytes)\n", a->info.unknown.length);
        }
    }
}

void print_methods(const MethodInfo *methods, uint16_t methods_count,
                   const ConstantPoolEntry *cp)
{
    printf("Methods (count: %u)\n", methods_count);
    printf("{\n");

    if (methods_count == 0) {
        printf("\t (no methods)\n");
        printf("}\n");
        return;
    }

    for (uint16_t i = 0; i < methods_count; i++) {
        const MethodInfo *m = &methods[i];
        const char *name       = cp_get_utf8(cp, m->name_index);
        const char *descriptor = cp_get_utf8(cp, m->descriptor_index);
        char flags_str[128];
        method_flags_to_string(m->access_flags, flags_str, sizeof(flags_str));

        printf("\t [%u] MethodInfo\n", i + 1);
        printf("\t\t Name: \t\t\t\t ConstantPoolInfo #%u <%s>\n",
               m->name_index, name ? name : "?");
        printf("\t\t Descriptor: \t\t\t ConstantPoolInfo #%u <%s>\n",
               m->descriptor_index, descriptor ? descriptor : "?");
        printf("\t\t Access Flags: \t\t\t 0x%04X [%s]\n",
               m->access_flags, flags_str);
        printf("\t\t Attributes count: \t\t %u\n", m->attributes_count);

        if (m->attributes_count > 0)
            print_method_attributes(m->attributes, m->attributes_count, cp);

        if (i + 1 < methods_count)
            printf("\n");
    }

    printf("}\n");
}

void free_methods(MethodInfo *methods, uint16_t methods_count)
{
    if (!methods) return;

    for (uint16_t i = 0; i < methods_count; i++) {
        MethodInfo *m = &methods[i];
        if (!m->attributes) continue;

        for (uint16_t j = 0; j < m->attributes_count; j++) {
            MethodAttributeInfo *a = &m->attributes[j];

            if (a->tag == MATTR_CODE) {
                free(a->info.code.code_bytes);
                free(a->info.code.exception_table);
                if (a->info.code.attributes) {
                    for (uint16_t k = 0; k < a->info.code.attributes_count; k++)
                        free(a->info.code.attributes[k].raw_bytes);
                    free(a->info.code.attributes);
                }
            } else if (a->tag == MATTR_EXCEPTIONS) {
                free(a->info.exceptions.exception_index_table);
            } else {
                free(a->info.unknown.data);
            }
        }
        free(m->attributes);
    }
    free(methods);
}