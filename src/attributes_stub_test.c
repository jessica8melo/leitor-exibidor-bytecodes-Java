#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "attributes.h"

static uint16_t read_u2(const uint8_t *data, uint32_t *offset) {
    uint16_t value = ((uint16_t)data[*offset] << 8) | data[*offset + 1];
    *offset += 2;
    return value;
}

static uint32_t read_u4(const uint8_t *data, uint32_t *offset) {
    uint32_t value = ((uint32_t)data[*offset] << 24) |
                     ((uint32_t)data[*offset + 1] << 16) |
                     ((uint32_t)data[*offset + 2] << 8) |
                     (uint32_t)data[*offset + 3];
    *offset += 4;
    return value;
}

static char **g_utf8_strings = NULL;
static uint16_t g_cp_count = 0;

static char *dup_utf8_bytes(const uint8_t *data, uint16_t len) {
    char *str = malloc(len + 1);
    if (!str) {
        fprintf(stderr, "sem memoria para utf8\n");
        exit(1);
    }
    memcpy(str, data, len);
    str[len] = '\0';
    return str;
}

static void parse_constant_pool(const uint8_t *data, uint32_t data_len, uint32_t *offset) {
    g_cp_count = read_u2(data, offset);
    g_utf8_strings = calloc(g_cp_count, sizeof(char *));
    if (!g_utf8_strings) {
        fprintf(stderr, "sem memoria para o constant pool\n");
        exit(1);
    }

    for (uint16_t i = 1; i < g_cp_count; i++) {
        uint8_t tag = data[*offset];
        (*offset)++;

        switch (tag) {
            case 1: {
                uint16_t len = read_u2(data, offset);
                if (*offset + len > data_len) {
                    fprintf(stderr, "Utf8 fora do limite\n");
                    exit(1);
                }
                g_utf8_strings[i] = dup_utf8_bytes(data + *offset, len);
                *offset += len;
                break;
            }
            case 3:
            case 4:
                *offset += 4;
                break;
            case 5:
            case 6:
                *offset += 8;
                i++;
                break;
            case 7:
            case 8:
            case 16:
            case 19:
            case 20:
                *offset += 2;
                break;
            case 9:
            case 10:
            case 11:
            case 12:
                *offset += 4;
                break;
            case 15:
                *offset += 3;
                break;
            case 17:
            case 18:
                *offset += 4;
                break;
            default:
                fprintf(stderr, "tag desconhecida: %u\n", tag);
                exit(1);
        }
    }
}

static void free_constant_pool_strings(void) {
    if (!g_utf8_strings) return;
    for (uint16_t i = 1; i < g_cp_count; i++) {
        free(g_utf8_strings[i]);
    }
    free(g_utf8_strings);
    g_utf8_strings = NULL;
    g_cp_count = 0;
}

static void class_flags_to_string(uint16_t flags, char *buf, size_t buf_size) {
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

    for (size_t i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
        if (flags & table[i].mask) {
            if (buf[0] != '\0') {
                strncat(buf, " ", buf_size - strlen(buf) - 1);
            }
            strncat(buf, table[i].name, buf_size - strlen(buf) - 1);
        }
    }

    if (buf[0] == '\0') {
        strncpy(buf, "(none)", buf_size - 1);
    }
}

static void skip_fields(const uint8_t *data, uint32_t data_len, uint32_t *offset) {
    uint16_t count = read_u2(data, offset);
    for (uint16_t i = 0; i < count; i++) {
        *offset += 6;
        uint16_t attr_count = read_u2(data, offset);
        for (uint16_t j = 0; j < attr_count; j++) {
            *offset += 2;
            uint32_t len = read_u4(data, offset);
            if (*offset + len > data_len) {
                fprintf(stderr, "field attr fora do limite\n");
                exit(1);
            }
            *offset += len;
        }
    }
}

static void skip_methods(const uint8_t *data, uint32_t data_len, uint32_t *offset) {
    uint16_t count = read_u2(data, offset);
    for (uint16_t i = 0; i < count; i++) {
        *offset += 6;
        uint16_t attr_count = read_u2(data, offset);
        for (uint16_t j = 0; j < attr_count; j++) {
            *offset += 2;
            uint32_t len = read_u4(data, offset);
            if (*offset + len > data_len) {
                fprintf(stderr, "method attr fora do limite\n");
                exit(1);
            }
            *offset += len;
        }
    }
}

const char *cp_get_utf8(const ConstantPoolEntry *cp, uint16_t index) {
    (void)cp;
    if (index == 0 || index >= g_cp_count) return "?";
    if (!g_utf8_strings[index]) return "?";
    return g_utf8_strings[index];
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.class>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len_long = ftell(f);
    rewind(f);

    if (len_long <= 0) {
        fprintf(stderr, "arquivo vazio\n");
        fclose(f);
        return 1;
    }

    uint32_t data_len = (uint32_t)len_long;
    uint8_t *data = malloc(data_len);
    if (!data) {
        perror("malloc");
        fclose(f);
        return 1;
    }

    if (fread(data, 1, data_len, f) != data_len) {
        fprintf(stderr, "erro ao ler arquivo\n");
        free(data);
        fclose(f);
        return 1;
    }
    fclose(f);

    uint32_t offset = 0;
    uint32_t magic = read_u4(data, &offset);
    if (magic != 0xCAFEBABE) {
        fprintf(stderr, "nao e um classfile\n");
        free(data);
        return 1;
    }

    offset += 4;
    parse_constant_pool(data, data_len, &offset);

    uint16_t class_access_flags = read_u2(data, &offset);
    offset += 4;
    uint16_t iface_count = read_u2(data, &offset);
    char class_flags_str[128];
    class_flags_to_string(class_access_flags, class_flags_str, sizeof(class_flags_str));
    printf("Access Flags: 0x%04X [%s]\n", class_access_flags, class_flags_str);

    offset += iface_count * 2;
    skip_fields(data, data_len, &offset);
    skip_methods(data, data_len, &offset);

    uint16_t class_attr_count = read_u2(data, &offset);
    printf("class_attr_count=%u\n", class_attr_count);

    ClassAttributeInfo *attrs = parse_class_attributes(data, data_len, &offset, class_attr_count, NULL);
    if (!attrs) {
        fprintf(stderr, "parse_class_attributes retornou NULL\n");
        free_constant_pool_strings();
        free(data);
        return 1;
    }

    for (uint16_t i = 0; i < class_attr_count; i++) {
        if (attrs[i].tag == ATTR_CLASS_SOURCE_FILE) {
            printf("SourceFile index = %u\n", attrs[i].info.source_file.sourcefile_index);
            printf("Nome do atributo: %s\n", cp_get_utf8(NULL, attrs[i].attribute_name_index));
            printf("SourceFile: %s\n", cp_get_utf8(NULL, attrs[i].info.source_file.sourcefile_index));
        }
    }

    free_class_attributes(attrs, class_attr_count);
    free_constant_pool_strings();
    free(data);
    return 0;
}
