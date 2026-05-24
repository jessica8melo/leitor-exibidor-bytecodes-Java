#ifndef BYTECODE_H
#define BYTECODE_H

#include "classfile.h"

typedef struct{

    char *mnemonic;
    int operands;

} Opcode;

extern Opcode opcodeTable[256];

void print_bytecodes(
    ClassFile *cf,
    u1 *code,
    u4 code_length,
    FILE *out
);

#endif