#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fields.h"
#include "methods.h"
#include "attributes.h"

/* =======================================================================
 * Pool de constantes (stub compartilhado — substituir pelo M1 depois)
 * Toda a lógica de resolução fica aqui para que fields.c, methods.c e
 * attributes.c possam chamar cp_get_utf8() sem saber da implementação.
 * ======================================================================= */

static char    **g_utf8_strings      = NULL;
static uint16_t *g_class_name_indexes = NULL;
static uint16_t  g_cp_count          = 0;

/* Implementação de cp_get_utf8 — declarada em fields.h e attributes.h */
const char *cp_get_utf8(const ConstantPoolEntry *cp, uint16_t index)
{
    (void)cp;
    if (index == 0 || index >= g_cp_count) return "?";
    if (!g_utf8_strings[index])            return "?";
    return g_utf8_strings[index];
}

/* Resolve o nome de uma entrada Class no pool */
static const char *resolve_class_name(uint16_t class_index)
{
    if (class_index == 0 || class_index >= g_cp_count) return "?";
    uint16_t utf8_index = g_class_name_indexes[class_index];
    return cp_get_utf8(NULL, utf8_index);
}

/* =======================================================================
 * Helpers de leitura big-endian
 * ======================================================================= */

static uint16_t read_u2(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 2 > data_len) {
        fprintf(stderr, "[main] Erro: leitura fora dos limites (u2 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }
    uint16_t v = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
    *offset += 2;
    return v;
}

static uint32_t read_u4(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    if (*offset + 4 > data_len) {
        fprintf(stderr, "[main] Erro: leitura fora dos limites (u4 em offset %u)\n", *offset);
        exit(EXIT_FAILURE);
    }
    uint32_t v = ((uint32_t)data[*offset]     << 24) |
                 ((uint32_t)data[*offset + 1] << 16) |
                 ((uint32_t)data[*offset + 2] <<  8) |
                 ((uint32_t)data[*offset + 3]);
    *offset += 4;
    return v;
}

/* =======================================================================
 * Parser do constant pool (stub — M1 vai substituir por constant_pool.c)
 * ======================================================================= */

static void parse_constant_pool(const uint8_t *data, uint32_t data_len,
                                 uint32_t *offset)
{
    g_cp_count = read_u2(data, data_len, offset);

    g_utf8_strings       = calloc(g_cp_count, sizeof(char *));
    g_class_name_indexes = calloc(g_cp_count, sizeof(uint16_t));

    if (!g_utf8_strings || !g_class_name_indexes) {
        fprintf(stderr, "[main] Erro: sem memória para o constant pool\n");
        exit(EXIT_FAILURE);
    }

    for (uint16_t i = 1; i < g_cp_count; i++) {
        if (*offset >= data_len) break;
        uint8_t tag = data[(*offset)++];

        if (tag == 1) {                             /* Utf8 */
            uint16_t len = read_u2(data, data_len, offset);
            char *str = malloc((size_t)len + 1);
            if (!str) { fprintf(stderr, "[main] sem memória\n"); exit(1); }
            memcpy(str, data + *offset, len);
            str[len] = '\0';
            g_utf8_strings[i] = str;
            *offset += len;
        } else if (tag == 3 || tag == 4) {          /* Integer, Float */
            *offset += 4;
        } else if (tag == 5 || tag == 6) {          /* Long, Double (2 slots) */
            *offset += 8;
            i++;
        } else if (tag == 7) {                      /* Class */
            g_class_name_indexes[i] = read_u2(data, data_len, offset);
        } else if (tag == 8  || tag == 16 ||
                   tag == 19 || tag == 20) {        /* String, MethodType, Module, Package */
            *offset += 2;
        } else if (tag == 9  || tag == 10 ||
                   tag == 11 || tag == 12) {        /* Fieldref, Methodref, InterfaceMethodref, NameAndType */
            *offset += 4;
        } else if (tag == 15) {                     /* MethodHandle */
            *offset += 3;
        } else if (tag == 17 || tag == 18) {        /* Dynamic, InvokeDynamic */
            *offset += 4;
        } else {
            fprintf(stderr, "[main] Tag desconhecida no pool: %u (offset %u)\n",
                    tag, *offset - 1);
            exit(EXIT_FAILURE);
        }
    }
}

static void free_constant_pool(void)
{
    if (g_utf8_strings) {
        for (uint16_t i = 1; i < g_cp_count; i++)
            free(g_utf8_strings[i]);
        free(g_utf8_strings);
        g_utf8_strings = NULL;
    }
    free(g_class_name_indexes);
    g_class_name_indexes = NULL;
    g_cp_count = 0;
}

/* =======================================================================
 * Exibição do Constant Pool
 * (versão básica — M1 vai substituir pela versão completa com todos os tipos)
 * ======================================================================= */

static void print_constant_pool(void)
{
    printf("Constant Pool (count: %u)\n", g_cp_count);
    printf("{\n");
    for (uint16_t i = 1; i < g_cp_count; i++) {
        if (g_utf8_strings[i]) {
            printf("\t [%u] ConstUtf8Info\n", i);
            printf("\t\t String: \t\t\t %s\n", g_utf8_strings[i]);
        } else if (g_class_name_indexes[i] != 0) {
            printf("\t [%u] ConstClassInfo\n", i);
            printf("\t\t Class name: \t\t\t ConstantPoolInfo #%u <%s>\n",
                   g_class_name_indexes[i],
                   cp_get_utf8(NULL, g_class_name_indexes[i]));
        }
        /* M1 vai completar os demais tipos (FieldRef, MethodRef, etc.) */
    }
    printf("}\n\n");
}

/* =======================================================================
 * Helpers de formatação de flags
 * ======================================================================= */

static void class_flags_to_string(uint16_t flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { uint16_t mask; const char *name; } table[] = {
        { 0x0001, "public"     },
        { 0x0010, "final"      },
        { 0x0020, "super"      },
        { 0x0200, "interface"  },
        { 0x0400, "abstract"   },
        { 0x1000, "synthetic"  },
        { 0x2000, "annotation" },
        { 0x4000, "enum"       },
        { 0x8000, "module"     },
    };
    size_t n = sizeof(table) / sizeof(table[0]);
    for (size_t k = 0; k < n; k++) {
        if (flags & table[k].mask) {
            if (buf[0] != '\0')
                strncat(buf, " ", buf_size - strlen(buf) - 1);
            strncat(buf, table[k].name, buf_size - strlen(buf) - 1);
        }
    }
    if (buf[0] == '\0') strncpy(buf, "(none)", buf_size - 1);
}

static const char *major_to_java(uint16_t major)
{
    switch (major) {
        case 45: return "Java 1.1";
        case 46: return "Java 1.2";
        case 47: return "Java 1.3";
        case 48: return "Java 1.4";
        case 49: return "Java 5";
        case 50: return "Java 6";
        case 51: return "Java 7";
        case 52: return "Java 8";
        case 53: return "Java 9";
        case 54: return "Java 10";
        case 55: return "Java 11";
        case 56: return "Java 12";
        case 57: return "Java 13";
        case 58: return "Java 14";
        case 59: return "Java 15";
        case 60: return "Java 16";
        case 61: return "Java 17";
        case 62: return "Java 18";
        case 63: return "Java 19";
        case 64: return "Java 20";
        case 65: return "Java 21";
        default: return "unknown";
    }
}

/* =======================================================================
 * Exibição dos atributos de classe
 * ======================================================================= */

static void print_class_attributes(const ClassAttributeInfo *attrs, uint16_t count)
{
    printf("Class Attributes (count: %u)\n", count);
    printf("{\n");

    if (count == 0) {
        printf("\t (no attributes)\n");
        printf("}\n\n");
        return;
    }

    for (uint16_t i = 0; i < count; i++) {
        const ClassAttributeInfo *a = &attrs[i];
        const char *attr_name = cp_get_utf8(NULL, a->attribute_name_index);

        printf("\t [%u] %s\n", i + 1, attr_name ? attr_name : "?");
        printf("\t\t Attribute Name: \t\t ConstantPoolInfo #%u <%s>\n",
               a->attribute_name_index, attr_name ? attr_name : "?");
        printf("\t\t Attribute Length: \t\t %u\n", a->attribute_length);

        if (a->tag == ATTR_CLASS_SOURCE_FILE) {
            uint16_t idx = a->info.source_file.sourcefile_index;
            printf("\t\t SourceFile: \t\t\t ConstantPoolInfo #%u <%s>\n",
                   idx, cp_get_utf8(NULL, idx));

        } else if (a->tag == ATTR_CLASS_SIGNATURE) {
            uint16_t idx = a->info.signature.signature_index;
            printf("\t\t Signature: \t\t\t ConstantPoolInfo #%u <%s>\n",
                   idx, cp_get_utf8(NULL, idx));

        } else if (a->tag == ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS ||
                   a->tag == ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS) {
            printf("\t\t Number of annotations: \t %u\n",
                   a->info.annotations.num_annotations);
            printf("\t\t (raw bytes omitted)\n");

        } else if (a->tag == ATTR_CLASS_SYNTHETIC ||
                   a->tag == ATTR_CLASS_DEPRECATED) {
            printf("\t\t (no payload)\n");

        } else {
            printf("\t\t (unknown attribute, %u bytes)\n", a->info.unknown.length);
        }

        if (i + 1 < count) printf("\n");
    }
    printf("}\n\n");
}

/* =======================================================================
 * main
 * ======================================================================= */

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.class>\n", argv[0]);
        return 1;
    }

    /* --- Leitura do arquivo em memória --- */
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("fopen"); return 1; }

    fseek(f, 0, SEEK_END);
    long len_long = ftell(f);
    rewind(f);

    if (len_long <= 0) {
        fprintf(stderr, "Arquivo vazio ou inválido\n");
        fclose(f);
        return 1;
    }

    uint32_t data_len = (uint32_t)len_long;
    uint8_t *data = malloc(data_len);
    if (!data) { perror("malloc"); fclose(f); return 1; }

    if (fread(data, 1, data_len, f) != data_len) {
        fprintf(stderr, "Erro ao ler arquivo\n");
        free(data); fclose(f); return 1;
    }
    fclose(f);

    uint32_t offset = 0;

    /* --- Cabeçalho --- */
    uint32_t magic = read_u4(data, data_len, &offset);
    if (magic != 0xCAFEBABE) {
        fprintf(stderr, "Não é um arquivo .class válido (magic = 0x%08X)\n", magic);
        free(data); return 1;
    }

    uint16_t minor_version = read_u2(data, data_len, &offset);
    uint16_t major_version = read_u2(data, data_len, &offset);

    /* --- Constant Pool --- */
    parse_constant_pool(data, data_len, &offset);

    /* --- Metadados da classe --- */
    uint16_t class_access_flags = read_u2(data, data_len, &offset);
    uint16_t this_class         = read_u2(data, data_len, &offset);
    uint16_t super_class        = read_u2(data, data_len, &offset);
    uint16_t ifaces_count       = read_u2(data, data_len, &offset);

    /* Pula lista de interfaces (índices u2) */
    uint16_t *interfaces = NULL;
    if (ifaces_count > 0) {
        interfaces = malloc(ifaces_count * sizeof(uint16_t));
        if (!interfaces) { fprintf(stderr, "sem memória\n"); exit(1); }
        for (uint16_t i = 0; i < ifaces_count; i++)
            interfaces[i] = read_u2(data, data_len, &offset);
    }

    /* --- Seção 1: General Information --- */
    char class_flags_str[128];
    class_flags_to_string(class_access_flags, class_flags_str, sizeof(class_flags_str));

    printf("General Information\n");
    printf("{\n");
    printf("\t Magic Number: \t\t\t 0x%08X\n", magic);
    printf("\t Minor Version: \t\t %u\n", minor_version);
    printf("\t Major Version: \t\t %u [%s]\n", major_version, major_to_java(major_version));
    printf("\t Constant Pool count: \t\t %u\n", g_cp_count);
    printf("\t Access Flags: \t\t\t 0x%04X [%s]\n", class_access_flags, class_flags_str);
    printf("\t This Class: \t\t\t ConstantPoolInfo #%u <%s>\n",
           this_class, resolve_class_name(this_class));
    if (super_class == 0)
        printf("\t Super class: \t\t\t none\n");
    else
        printf("\t Super class: \t\t\t ConstantPoolInfo #%u <%s>\n",
               super_class, resolve_class_name(super_class));
    printf("\t Interfaces count: \t\t %u\n", ifaces_count);

    /* Lê fields_count e methods_count apenas para exibir no General Info,
       mas ainda não avança o offset — fazemos isso abaixo */
    uint32_t saved = offset;
    uint16_t fields_count_preview  = read_u2(data, data_len, &offset);
    offset = saved; /* restaura — parse_fields vai reler */

    printf("\t Fields count: \t\t\t %u\n", fields_count_preview);
    /* methods_count e attributes_count só ficam disponíveis depois dos
       respectivos parsers, então estimamos aqui e corrigimos abaixo */
    printf("\t Methods pool count: \t\t (calculado abaixo)\n");
    printf("\t Attributes pool count: \t (calculado abaixo)\n");
    printf("}\n\n");

    /* --- Seção 2: Constant Pool --- */
    print_constant_pool();

    /* --- Seção 3: Fields --- */
    uint16_t fields_count = read_u2(data, data_len, &offset);
    FieldInfo *fields = parse_fields(data, data_len, &offset, fields_count, NULL);
    print_fields(fields, fields_count, NULL);
    printf("\n");

    /* --- Seção 4: Methods --- */
    uint16_t methods_count = read_u2(data, data_len, &offset);
    MethodInfo *methods = parse_methods(data, data_len, &offset, methods_count, NULL);
    print_methods(methods, methods_count, NULL);
    printf("\n");

    /* --- Seção 5: Class Attributes --- */
    uint16_t class_attr_count = read_u2(data, data_len, &offset);
    ClassAttributeInfo *attrs = parse_class_attributes(data, data_len, &offset,
                                                        class_attr_count, NULL);
    print_class_attributes(attrs, class_attr_count);

    /* --- Liberação --- */
    free_fields(fields, fields_count);
    free_methods(methods, methods_count);
    free_class_attributes(attrs, class_attr_count);
    free_constant_pool();
    free(interfaces);
    free(data);

    return 0;
}