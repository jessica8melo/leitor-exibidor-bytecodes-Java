#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "attributes.h"

static uint16_t read_u2(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 2 > data_len) {
        fprintf(stderr, "[attributes] Erro: leitura fora dos limites (u2 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }

    uint16_t value = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
    *offset += 2;
    return value;
}

static uint32_t read_u4(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 4 > data_len) {
        fprintf(stderr, "[attributes] Erro: leitura fora dos limites (u4 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }

    uint32_t value = ((uint32_t)data[*offset] << 24) |
                     ((uint32_t)data[*offset + 1] << 16) |
                     ((uint32_t)data[*offset + 2] << 8) |
                     (uint32_t)data[*offset + 3];
    *offset += 4;
    return value;
}

static ClassAttributeTag resolve_class_attr_tag(const char *name)
{
    if (!name) return ATTR_CLASS_UNKNOWN;
    if (strcmp(name, "SourceFile") == 0) return ATTR_CLASS_SOURCE_FILE;
    if (strcmp(name, "Signature") == 0) return ATTR_CLASS_SIGNATURE;
    if (strcmp(name, "Synthetic") == 0) return ATTR_CLASS_SYNTHETIC;
    if (strcmp(name, "Deprecated") == 0) return ATTR_CLASS_DEPRECATED;
    if (strcmp(name, "RuntimeVisibleAnnotations") == 0) return ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS;
    if (strcmp(name, "RuntimeInvisibleAnnotations") == 0) return ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS;
    return ATTR_CLASS_UNKNOWN;
}

static void parse_unknown_attribute(const uint8_t *data, uint32_t *offset, ClassAttributeInfo *attr)
{
    attr->info.unknown.length = attr->attribute_length;
    if (attr->attribute_length > 0) {
        attr->info.unknown.data = malloc(attr->attribute_length);
        if (!attr->info.unknown.data) {
            fprintf(stderr, "[attributes] Erro: sem memória para atributo desconhecido\n");
            exit(EXIT_FAILURE);
        }
        memcpy(attr->info.unknown.data, data + *offset, attr->attribute_length);
        *offset += attr->attribute_length;
    } else {
        attr->info.unknown.data = NULL;
    }
}

static void parse_annotations_attribute(const uint8_t *data, uint32_t data_len,
                                        uint32_t *offset, ClassAttributeInfo *attr)
{
    attr->info.annotations.num_annotations = read_u2(data, data_len, offset);
    uint32_t remaining = attr->attribute_length - 2;
    attr->info.annotations.raw_length = remaining;

    if (remaining > 0) {
        attr->info.annotations.raw_bytes = malloc(remaining);
        if (!attr->info.annotations.raw_bytes) {
            fprintf(stderr, "[attributes] Erro: sem memória para annotations\n");
            exit(EXIT_FAILURE);
        }
        memcpy(attr->info.annotations.raw_bytes, data + *offset, remaining);
        *offset += remaining;
    } else {
        attr->info.annotations.raw_bytes = NULL;
    }
}

static void parse_class_attribute(const uint8_t *data, uint32_t data_len,
                                  uint32_t *offset, ClassAttributeInfo *attr,
                                  const ConstantPoolEntry *cp)
{
    attr->attribute_name_index = read_u2(data, data_len, offset);
    attr->attribute_length = read_u4(data, data_len, offset);

    uint32_t end_offset = *offset + attr->attribute_length;
    const char *attr_name = cp_get_utf8(cp, attr->attribute_name_index);
    attr->tag = resolve_class_attr_tag(attr_name);

    if (attr->tag == ATTR_CLASS_SOURCE_FILE) {
        attr->info.source_file.sourcefile_index = read_u2(data, data_len, offset);
    } else if (attr->tag == ATTR_CLASS_SIGNATURE) {
        attr->info.signature.signature_index = read_u2(data, data_len, offset);
    } else if (attr->tag == ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS ||
               attr->tag == ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS) {
        parse_annotations_attribute(data, data_len, offset, attr);
    } else if (attr->tag == ATTR_CLASS_SYNTHETIC || attr->tag == ATTR_CLASS_DEPRECATED) {
        /* sem payload */
    } else {
        parse_unknown_attribute(data, offset, attr);
    }

    if (*offset != end_offset) {
        fprintf(stderr,
                "[attributes] Aviso: offset inconsistente após atributo '%s' (esperado %u, atual %u)\n",
                attr_name ? attr_name : "?", end_offset, *offset);
        *offset = end_offset;
    }
}

ClassAttributeInfo *parse_class_attributes(const uint8_t *data, uint32_t data_len,
                                           uint32_t *offset, uint16_t attributes_count,
                                           const ConstantPoolEntry *cp)
{
    if (attributes_count == 0) return NULL;

    ClassAttributeInfo *attributes = calloc(attributes_count, sizeof(ClassAttributeInfo));
    if (!attributes) {
        fprintf(stderr, "[attributes] Erro: sem memória para atributos da classe\n");
        exit(EXIT_FAILURE);
    }

    for (uint16_t i = 0; i < attributes_count; i++)
        parse_class_attribute(data, data_len, offset, &attributes[i], cp);

    return attributes;
}

void free_class_attributes(ClassAttributeInfo *attributes, uint16_t attributes_count)
{
    if (!attributes) return;

    for (uint16_t i = 0; i < attributes_count; i++) {
        ClassAttributeInfo *attr = &attributes[i];

        if (attr->tag == ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS ||
            attr->tag == ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS) {
            free(attr->info.annotations.raw_bytes);
        } else if (attr->tag == ATTR_CLASS_UNKNOWN) {
            free(attr->info.unknown.data);
        }
    }

    free(attributes);
}
