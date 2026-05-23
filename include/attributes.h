#ifndef ATTRIBUTES_H
#define ATTRIBUTES_H

#include <stdint.h>
#include <stdlib.h>
#include "constant_pool.h"

typedef struct {
    uint16_t sourcefile_index;
} SourceFile_attribute;

typedef struct {
    uint16_t signature_index;
} ClassSignature_attribute;

typedef struct {
    uint16_t  num_annotations;
    uint8_t  *raw_bytes;
    uint32_t  raw_length;
} ClassAnnotations_attribute;

typedef enum {
    ATTR_CLASS_SOURCE_FILE = 0,
    ATTR_CLASS_SIGNATURE,
    ATTR_CLASS_SYNTHETIC,
    ATTR_CLASS_DEPRECATED,
    ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS,
    ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS,
    ATTR_CLASS_UNKNOWN
} ClassAttributeTag;

typedef struct {
    uint16_t          attribute_name_index;
    uint32_t          attribute_length;
    ClassAttributeTag tag;
    union {
        SourceFile_attribute   source_file;
        ClassSignature_attribute    signature;
        ClassAnnotations_attribute  annotations;
        struct {
            uint8_t *data;
            uint32_t length;
        } unknown;
    } info;
} ClassAttributeInfo;

ClassAttributeInfo *parse_class_attributes(const uint8_t *data, uint32_t data_len,
                                           uint32_t *offset, uint16_t attributes_count,
                                           const ConstantPoolEntry *cp);

void free_class_attributes(ClassAttributeInfo *attributes, uint16_t attributes_count);

#endif /* ATTRIBUTES_H */
