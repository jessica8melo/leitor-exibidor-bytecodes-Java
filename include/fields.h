#ifndef FIELDS_H
#define FIELDS_H

#include <stdint.h>
#include <stdlib.h>
#include "constant_pool.h"

typedef struct {
    uint16_t constantvalue_index;
} ConstantValue_attribute;

typedef struct {
    uint16_t signature_index;
} Signature_attribute;

typedef struct {
    uint16_t  num_annotations;
    uint8_t  *raw_bytes;
    uint32_t  raw_length;
} Annotations_attribute;

typedef enum {
    ATTR_CONSTANT_VALUE = 0,
    ATTR_SYNTHETIC,
    ATTR_DEPRECATED,
    ATTR_SIGNATURE,
    ATTR_RUNTIME_VISIBLE_ANNOTATIONS,
    ATTR_RUNTIME_INVISIBLE_ANNOTATIONS,
    ATTR_UNKNOWN
} FieldAttributeTag;

typedef struct {
    uint16_t          attribute_name_index;
    uint32_t          attribute_length;
    FieldAttributeTag tag;
    union {
        ConstantValue_attribute constant_value;
        Signature_attribute     signature;
        Annotations_attribute   annotations;
        struct {
            uint8_t  *data;
            uint32_t  length;
        } unknown;
    } info;
} FieldAttributeInfo;

typedef struct {
    uint16_t           access_flags;
    uint16_t           name_index;
    uint16_t           descriptor_index;
    uint16_t           attributes_count;
    FieldAttributeInfo *attributes;
} FieldInfo;

#define ACC_PUBLIC    0x0001
#define ACC_PRIVATE   0x0002
#define ACC_PROTECTED 0x0004
#define ACC_STATIC    0x0008
#define ACC_FINAL     0x0010
#define ACC_VOLATILE  0x0040
#define ACC_TRANSIENT 0x0080
#define ACC_SYNTHETIC 0x1000
#define ACC_ENUM      0x4000

FieldInfo *parse_fields(const uint8_t *data, uint32_t data_len,
                        uint32_t *offset, uint16_t fields_count,
                        const ConstantPoolEntry *cp);

void print_fields(const FieldInfo *fields, uint16_t fields_count,
                  const ConstantPoolEntry *cp);

void free_fields(FieldInfo *fields, uint16_t fields_count);

#endif /* FIELDS_H */
