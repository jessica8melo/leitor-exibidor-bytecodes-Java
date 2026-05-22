// structs e funções de methods
#ifndef METHODS_H
#define METHODS_H

#include <stdint.h>
#include <stdlib.h>
#include "fields.h"   

typedef struct {
    uint16_t  max_stack;
    uint16_t  max_locals;
    uint32_t  code_length;
    uint8_t  *code_bytes;      

    uint16_t  exception_table_length;
    struct {
        uint16_t start_pc;
        uint16_t end_pc;
        uint16_t handler_pc;
        uint16_t catch_type;   
    } *exception_table;

    uint16_t  attributes_count;
    struct {
        uint16_t attribute_name_index;
        uint32_t attribute_length;
        uint8_t *raw_bytes;    
    } *attributes;
} CodeAttribute;

typedef struct {
    uint16_t  number_of_exceptions;
    uint16_t *exception_index_table; 
} ExceptionsAttribute;

typedef struct {
    uint16_t signature_index;
} MethodSignature_attribute;

typedef struct {
    uint32_t  length;
    uint8_t  *raw_bytes;
} AnnotationDefault_attribute;

typedef enum {
    MATTR_CODE = 0,
    MATTR_EXCEPTIONS,
    MATTR_SIGNATURE,
    MATTR_DEPRECATED,
    MATTR_SYNTHETIC,
    MATTR_ANNOTATION_DEFAULT,
    MATTR_RUNTIME_VISIBLE_ANNOTATIONS,
    MATTR_RUNTIME_INVISIBLE_ANNOTATIONS,
    MATTR_RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS,
    MATTR_RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS,
    MATTR_UNKNOWN
} MethodAttributeTag;

typedef struct {
    uint16_t           attribute_name_index;
    uint32_t           attribute_length;
    MethodAttributeTag tag;
    union {
        CodeAttribute                 code;
        ExceptionsAttribute           exceptions;
        MethodSignature_attribute     signature;
        AnnotationDefault_attribute   annotation_default;
        struct {
            uint8_t  *data;
            uint32_t  length;
        } unknown;
    } info;
} MethodAttributeInfo;

typedef struct {
    uint16_t            access_flags;
    uint16_t            name_index;
    uint16_t            descriptor_index;
    uint16_t            attributes_count;
    MethodAttributeInfo *attributes;
} MethodInfo;

#define MACC_PUBLIC       0x0001
#define MACC_PRIVATE      0x0002
#define MACC_PROTECTED    0x0004
#define MACC_STATIC       0x0008
#define MACC_FINAL        0x0010
#define MACC_SYNCHRONIZED 0x0020
#define MACC_BRIDGE       0x0040
#define MACC_VARARGS      0x0080
#define MACC_NATIVE       0x0100
#define MACC_ABSTRACT     0x0400
#define MACC_STRICT       0x0800
#define MACC_SYNTHETIC    0x1000

MethodInfo *parse_methods(const uint8_t *data, uint32_t data_len,
                          uint32_t *offset, uint16_t methods_count,
                          const ConstantPoolEntry *cp);

void print_methods(const MethodInfo *methods, uint16_t methods_count,
                   const ConstantPoolEntry *cp);

void free_methods(MethodInfo *methods, uint16_t methods_count);

#endif /* METHODS_H */