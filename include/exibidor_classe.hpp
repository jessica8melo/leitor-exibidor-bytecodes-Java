#ifndef EXIBIDOR_CLASSE_HPP
#define EXIBIDOR_CLASSE_HPP

#include "tipos_basicos.hpp"
#include "arquivo_classe.hpp"
#include "leitor_classe.hpp"
#include <cstdio>
#include <string>

/**
 * @file exibidor_classe.hpp
 * @brief Declaração do disassembler textual e exibidor de arquivos .class (Modo -r).
 */

/**
 * @brief Escreve o relatório formatado completo do arquivo de classe no arquivo ou fluxo de saída.
 * @param arq Ponteiro para a estrutura ArquivoClasse já carregada na memória.
 * @param saida Ponteiro FILE para o fluxo de saída (pode ser stdout ou arquivo aberto em modo escrita).
 */
void escrever_arquivo(ArquivoClasse* arq, FILE* saida);

/**
 * @brief Imprime as informações gerais da classe (versão, assinatura mágica, flags e índices).
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_info_geral(ArquivoClasse* arq);

/**
 * @brief Imprime todas as entradas do Pool de Constantes decodificadas e numeradas.
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_pool_constantes(ArquivoClasse* arq);

/**
 * @brief Imprime a lista de interfaces implementadas pela classe.
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_interfaces(ArquivoClasse* arq);

/**
 * @brief Imprime os campos (fields) da classe, seus tipos, descritores e atributos.
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_campos(ArquivoClasse* arq);

/**
 * @brief Imprime os métodos da classe, incluindo seus descritores, flags e bytecodes.
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_metodos(ArquivoClasse* arq);

/**
 * @brief Imprime os atributos de nível de classe (ex: SourceFile, InnerClasses).
 * @param arq Ponteiro para o ArquivoClasse.
 */
void escrever_atributos(ArquivoClasse* arq);

/**
 * @brief Imprime os detalhes de um atributo específico com indentação controlada.
 * @param a Estrutura AttributeInfo contendo o atributo a ser exibido.
 * @param idx Índice do atributo na lista do elemento pai.
 * @param pool Ponteiro para o Pool de Constantes da classe.
 * @param nivel Nível de indentação para formatação na saída textual.
 */
void escrever_atributo(AttributeInfo a, uint32_t idx, ConstantPoolInfo* pool, uint8_t nivel);

/**
 * @brief Disassebla e imprime o fluxo de instruções (bytecodes) do atributo Code de um método.
 * @param c Estrutura CodeAttribute contendo o vetor de bytes e tabelas de exceção.
 * @param pool Ponteiro para o Pool de Constantes.
 * @param nivel Nível de indentação na saída textual.
 */
void escrever_bytecode(CodeAttribute c, ConstantPoolInfo* pool, uint8_t nivel);

/**
 * @brief Retorna uma representação textual das flags de acesso combinadas.
 * @param access_flags Máscara de bits contendo as flags (public, static, final, etc.).
 * @return const char* String formatada listando as flags ativas.
 */
const char* flags_acesso_str(u2 access_flags);

/**
 * @brief Formata uma entrada do Pool de Constantes como string humanamente legível.
 * @param pool Ponteiro para o vetor do Pool de Constantes.
 * @param index Índice (1-indexed) da constante no pool.
 * @return const char* String resolvida contendo o nome ou valor da constante.
 */
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
