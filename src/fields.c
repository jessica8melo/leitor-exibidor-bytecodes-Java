#include <stdio.h>
#include <stdlib.h>
#include "fields.h"
#include "constant_pool.h"

static uint8_t read_u1(const uint8_t *data, uint32_t data_len,
                        uint32_t *offset) {
        if (*offset + 1 > data_len) {
            fprintf(stderr, "[fields] Erro: leitura fora dos limites (u1 em offset %u)\n", 
                    *offset);
            exit(EXIT_FAILURE);
        }
        return data[(*offset)++];
    }

static uint16_t read_u2(const uint8_t *data, uint32_t data_len,
                        uint32_t *offset) {
        if (*offset + 2 > data_len) {
            fprintf(stderr, "[fields] Erro: leitura fora dos limites (u2 em offset %u)\n", 
                    *offset);
            exit(EXIT_FAILURE);
        }
        uint16_t value = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
        *offset += 2;
        return value;
    }

static uint32_t read_u4(const uint8_t *data, uint32_t data_len,
                        uint32_t *offset) {
        if (*offset + 4 > data_len) {
            fprintf(stderr, "[fields] Erro: leitura fora dos limites (u4 em offset %u)\n", 
                    *offset);
            exit(EXIT_FAILURE);
        }
        uint32_t value = ((uint32_t)data[*offset]       << 24) | 
                         ((uint32_t)data[*offset + 1]   << 16) | 
                         ((uint32_t)data[*offset + 2]   <<  8) | 
                         ((uint32_t)data[*offset + 3]);
        *offset += 4;
        return value;
    }

static FieldAttributeTag resolve_attr_tag(const char *name) {
    if (strcmp(name, "ConstantValue") == 0) {
        return ATTR_CONSTANT_VALUE;
    } else if (strcmp(name, "Synthetic") == 0) {
        return ATTR_SYNTHETIC;
    } else if (strcmp(name, "Deprecated") == 0) {
        return ATTR_DEPRECATED;
    } else if (strcmp(name, "Signature") == 0) {
        return ATTR_SIGNATURE;
    } else if (strcmp(name, "RuntimeVisibleAnnotations") == 0) {
        return ATTR_RUNTIME_VISIBLE_ANNOTATIONS;
    } else if (strcmp(name, "RuntimeInvisibleAnnotations") == 0) {
        return ATTR_RUNTIME_INVISIBLE_ANNOTATIONS;
    } else {
        return ATTR_UNKNOWN;
    }
}

static void parse_field_attribute(const uint8_t *data, uint32_t data_len,
                                  uint32_t *offset,
                                  FieldAttributeInfo *attr,
                                  const ConstantPoolEntry *cp) {
    attr->attribute_name_index = read_u2(data, data_len, offset);
    attr->attribute_length     = read_u4(data, data_len, offset);

    const char *attr_name = cp_get_utf8(cp, attr->attribute_name_index);
    attr->tag = resolve_attr_tag(attr_name ? attr_name : "");

    switch (attr->tag) {
        case ATTR_CONSTANT_VALUE:
            attr->info.constant_value.constantvalue_index = read_u2(data, data_len, offset);
            break;
        
        case ATTR_SIGNATURE:
            attr->info.signature.signature_index = read_u2(data, data_len, offset);
            break;
        case ATTR_RUNTIME_VISIBLE_ANNOTATIONS:
        case ATTR_RUNTIME_INVISIBLE_ANNOTATIONS:
            attr->info.annotations.num_annotations = read_u2(data, data_len, offset);
            attr->info.annotations.raw_length = 
                attr->attribute_length - 2;
            if (attr->info.annotations.raw_length > 0) {
                attr->info.annotations.raw_bytes = 
                    malloc(attr->info.annotations.raw_length);
                if (!attr->info.annotations.raw_bytes) {
                    fprintf(stderr, "[fields] Erro: falha ao alocar memória para annotations\n");
                    exit(EXIT_FAILURE);
                }
                memcpy(attr->info.annotations.raw_bytes, 
                       data + *offset, 
                       attr->info.annotations.raw_length);
            } else {
                attr->info.annotations.raw_bytes = NULL;
            }
            *offset += attr->info.annotations.raw_length;
            break;

        case ATTR_SYNTHETIC:
        case ATTR_DEPRECATED:
            break;

        default:
            attr->info.unknown.length = attr->attribute_length;
            if (attr->attribute_length > 0) {
                attr->info.unknown.data = malloc(attr->attribute_length);
                if (!attr->info.unknown.data) {
                    fprintf(stderr, "[fields] Erro: falha ao alocar memória para atributo desconhecido\n");
                    exit(EXIT_FAILURE);
                }
                memcpy(attr->info.unknown.data, 
                       data + *offset, 
                       attr->attribute_length);
            } else {
                attr->info.unknown.data = NULL;
            }
            *offset += attr->attribute_length;
            break;
    }

    if (*offset != end_offset) {
        fprintf(stderr, "[fields] Aviso: atributo '%s' lido incorretamente (esperado %u bytes, lido %u bytes)\n",
                attr_name ? attr_name : "?", end_offset, *offset);
        *offset = end_offset;
    }
}

FieldInfo *parse_fields(const uint8_t *data, uint32_t data_len,
                        uint32_t *offset, uint16_t fields_count,
                        const ConstantPoolEntry *cp)
{
    if (fields_count == 0)
        return NULL;
 
    FieldInfo *fields = calloc(fields_count, sizeof(FieldInfo));
    if (!fields) {
        fprintf(stderr, "[fields] Erro: sem memória para fields\n");
        exit(EXIT_FAILURE);
    }
 
    for (uint16_t i = 0; i < fields_count; i++) {
        FieldInfo *f = &fields[i];
 
        f->access_flags      = read_u2(data, data_len, offset);
        f->name_index        = read_u2(data, data_len, offset);
        f->descriptor_index  = read_u2(data, data_len, offset);
        f->attributes_count  = read_u2(data, data_len, offset);
 
        if (f->attributes_count > 0) {
            f->attributes = calloc(f->attributes_count, sizeof(FieldAttributeInfo));
            if (!f->attributes) {
                fprintf(stderr, "[fields] Erro: sem memória para atributos do field %u\n", i);
                exit(EXIT_FAILURE);
            }
            for (uint16_t j = 0; j < f->attributes_count; j++) {
                parse_field_attribute(data, data_len, offset,
                                      &f->attributes[j], cp);
            }
        } else {
            f->attributes = NULL;
        }
    }
 
    return fields;
}
 
static void flags_to_string(uint16_t flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { uint16_t mask; const char *name; } table[] = {
        { ACC_PUBLIC,    "public"    },
        { ACC_PRIVATE,   "private"   },
        { ACC_PROTECTED, "protected" },
        { ACC_STATIC,    "static"    },
        { ACC_FINAL,     "final"     },
        { ACC_VOLATILE,  "volatile"  },
        { ACC_TRANSIENT, "transient" },
        { ACC_SYNTHETIC, "synthetic" },
        { ACC_ENUM,      "enum"      },
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
 
static void print_field_attributes(const FieldAttributeInfo *attrs,
                                   uint16_t count,
                                   const ConstantPoolEntry *cp)
{
    for (uint16_t j = 0; j < count; j++) {
        const FieldAttributeInfo *a = &attrs[j];
        const char *attr_name = cp_get_utf8(cp, a->attribute_name_index);
 
        printf("\t\t [%u] %s\n", j + 1, attr_name ? attr_name : "?");
 
        switch (a->tag) {
 
            case ATTR_CONSTANT_VALUE: {
                uint16_t cv_idx = a->info.constant_value.constantvalue_index;
                const char *cv_val = cp_get_utf8(cp, cv_idx);
                printf("\t\t\t Constant Value: \t\t ConstantPoolInfo #%u <%s>\n",
                       cv_idx, cv_val ? cv_val : "?");
                break;
            }
 
            case ATTR_SIGNATURE: {
                uint16_t sig_idx = a->info.signature.signature_index;
                const char *sig = cp_get_utf8(cp, sig_idx);
                printf("\t\t\t Signature: \t\t\t ConstantPoolInfo #%u <%s>\n",
                       sig_idx, sig ? sig : "?");
                break;
            }
 
            case ATTR_RUNTIME_VISIBLE_ANNOTATIONS:
            case ATTR_RUNTIME_INVISIBLE_ANNOTATIONS:
                printf("\t\t\t Number of annotations: \t %u\n",
                       a->info.annotations.num_annotations);
                printf("\t\t\t (raw bytes omitted)\n");
                break;
 
            case ATTR_SYNTHETIC:
                break;
 
            case ATTR_DEPRECATED:
                break;
 
            default:
                printf("\t\t\t (unknown attribute, %u bytes)\n",
                       a->info.unknown.length);
                break;
        }
    }
}
 
void print_fields(const FieldInfo *fields, uint16_t fields_count,
                  const ConstantPoolEntry *cp)
{
    printf("Fields (count: %u)\n", fields_count);
    printf("{\n");
 
    if (fields_count == 0) {
        printf("\t (no fields)\n");
        printf("}\n");
        return;
    }
 
    for (uint16_t i = 0; i < fields_count; i++) {
        const FieldInfo *f = &fields[i];
 
        const char *name       = cp_get_utf8(cp, f->name_index);
        const char *descriptor = cp_get_utf8(cp, f->descriptor_index);
 
        char flags_str[64];
        flags_to_string(f->access_flags, flags_str, sizeof(flags_str));
 
        printf("\t [%u] FieldInfo\n", i + 1);
        printf("\t\t Name: \t\t\t\t ConstantPoolInfo #%u <%s>\n",
               f->name_index, name ? name : "?");
        printf("\t\t Descriptor: \t\t\t ConstantPoolInfo #%u <%s>\n",
               f->descriptor_index, descriptor ? descriptor : "?");
        printf("\t\t Access Flags: \t\t\t 0x%04X [%s]\n",
               f->access_flags, flags_str);
        printf("\t\t Attributes count: \t\t %u\n", f->attributes_count);
 
        if (f->attributes_count > 0) {
            print_field_attributes(f->attributes, f->attributes_count, cp);
        }
 
        if (i + 1 < fields_count)
            printf("\n");
    }
 
    printf("}\n");
}
 
void free_fields(FieldInfo *fields, uint16_t fields_count)
{
    if (!fields) return;
 
    for (uint16_t i = 0; i < fields_count; i++) {
        FieldInfo *f = &fields[i];
        if (!f->attributes) continue;
 
        for (uint16_t j = 0; j < f->attributes_count; j++) {
            FieldAttributeInfo *a = &f->attributes[j];
            switch (a->tag) {
                case ATTR_RUNTIME_VISIBLE_ANNOTATIONS:
                case ATTR_RUNTIME_INVISIBLE_ANNOTATIONS:
                    free(a->info.annotations.raw_bytes);
                    break;
                case ATTR_UNKNOWN:
                    free(a->info.unknown.data);
                    break;
                default:
                    break;
            }
        }
        free(f->attributes);
    }
    free(fields);
}
 
