#ifndef EXIBIDOR_CLASSE_HPP
#define EXIBIDOR_CLASSE_HPP

#include "tipos_basicos.hpp"
#include "arquivo_classe.hpp"
#include "leitor_classe.hpp"
#include <cstdio>
#include <string>

void escrever_arquivo(ArquivoClasse* arq, FILE* saida);

void escrever_info_geral(ArquivoClasse* arq);
void escrever_pool_constantes(ArquivoClasse* arq);
void escrever_interfaces(ArquivoClasse* arq);
void escrever_campos(ArquivoClasse* arq);
void escrever_metodos(ArquivoClasse* arq);
void escrever_atributos(ArquivoClasse* arq);

void escrever_atributo(AttributeInfo a, uint32_t idx, ConstantPoolInfo* pool, uint8_t nivel);
void escrever_bytecode(CodeAttribute c, ConstantPoolInfo* pool, uint8_t nivel);

const char* flags_acesso_str(u2 access_flags);
const char* formatar_constante(ConstantPoolInfo* pool, u2 index);

static const std::string tabela_mnemonicos[] = {
     "nop", "aconst_null",
     "iconst_m1", "iconst_0", "iconst_1", "iconst_2",
     "iconst_3", "iconst_4", "iconst_5",
     "lconst_0", "lconst_1",
     "fconst_0", "fconst_1", "fconst_2",
     "dconst_0", "dconst_1",
     "bipush", "sipush",
     "ldc", "ldc_w", "ldc2_w",
     "iload", "lload", "fload", "dload", "aload",
     "iload_0", "iload_1", "iload_2", "iload_3",
     "lload_0", "lload_1", "lload_2", "lload_3",
     "fload_0", "fload_1", "fload_2", "fload_3",
     "dload_0", "dload_1", "dload_2", "dload_3",
     "aload_0", "aload_1", "aload_2", "aload_3",
     "iaload", "laload", "faload", "daload",
     "aaload", "baload", "caload", "saload",
     "istore", "lstore", "fstore", "dstore", "astore",
     "istore_0", "istore_1", "istore_2", "istore_3",
     "lstore_0", "lstore_1", "lstore_2", "lstore_3",
     "fstore_0", "fstore_1", "fstore_2", "fstore_3",
     "dstore_0", "dstore_1", "dstore_2", "dstore_3",
     "astore_0", "astore_1", "astore_2", "astore_3",
     "iastore", "lastore", "fastore", "dastore",
     "aastore", "bastore", "castore", "sastore",
     "pop", "pop2",
     "dup", "dup_x1", "dup_x2", "dup2", "dup2_x1", "dup2_x2",
     "swap",
     "iadd", "ladd", "fadd", "dadd",
     "isub", "lsub", "fsub", "dsub",
     "imul", "lmul", "fmul", "dmul",
     "idiv", "ldiv", "fdiv", "ddiv",
     "irem", "lrem", "frem", "drem",
     "ineg", "lneg", "fneg", "dneg",
     "ishl", "lshl", "ishr", "lshr",
     "iushr", "lushr",
     "iand", "land", "ior", "lor", "ixor", "lxor",
     "iinc",
     "i2l", "i2f", "i2d", "l2i", "l2f", "l2d",
     "f2i", "f2l", "f2d", "d2i", "d2l", "d2f",
     "i2b", "i2c", "i2s",
     "lcmp", "fcmpl", "fcmpg", "dcmpl", "dcmpg",
     "ifeq", "ifne", "iflt", "ifge", "ifgt", "ifle",
     "if_icmpeq", "if_icmpne", "if_icmplt", "if_icmpge", "if_icmpgt", "if_icmple",
     "if_acmpeq", "if_acmpne",
     "goto", "jsr", "ret",
     "tableswitch", "lookupswitch",
     "ireturn", "lreturn", "freturn", "dreturn", "areturn", "return",
     "getstatic", "putstatic", "getfield", "putfield",
     "invokevirtual", "invokespecial", "invokestatic", "invokeinterface",
     "UNUSED",
     "new", "newarray", "anewarray", "arraylength", "athrow",
     "checkcast", "instanceof",
     "monitorenter", "monitorexit",
     "wide", "multianewarray",
     "ifnull", "ifnonnull",
     "goto_w", "jsr_w"
};

#endif
