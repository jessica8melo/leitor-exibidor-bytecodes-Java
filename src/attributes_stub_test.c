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
static uint16_t *g_class_name_indexes = NULL;
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
    g_class_name_indexes = calloc(g_cp_count, sizeof(uint16_t));
    if (!g_utf8_strings || !g_class_name_indexes) {
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
                g_class_name_indexes[i] = read_u2(data, offset);
                break;
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

    free(g_class_name_indexes);
    g_class_name_indexes = NULL;
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

static const char *major_version_to_java_string(uint16_t major) {
    switch (major) {
        case 45: return "1.1";
        case 46: return "1.2";
        case 47: return "1.3";
        case 48: return "1.4";
        case 49: return "1.5";
        case 50: return "1.6";
        case 51: return "1.7";
        case 52: return "1.8";
        case 53: return "9";
        case 54: return "10";
        case 55: return "11";
        case 56: return "12";
        case 57: return "13";
        case 58: return "14";
        case 59: return "15";
        case 60: return "16";
        case 61: return "17";
        case 62: return "18";
        case 63: return "19";
        case 64: return "20";
        case 65: return "21";
        default: return "unknown";
    }
}

static const char *resolve_class_name(uint16_t class_index) {
    if (class_index == 0 || class_index >= g_cp_count) {
        return "?";
    }

    uint16_t utf8_index = g_class_name_indexes[class_index];
    if (utf8_index == 0 || utf8_index >= g_cp_count) {
        return "?";
    }

    return cp_get_utf8(NULL, utf8_index);
}

static void skip_interfaces(uint32_t *offset, uint16_t iface_count) {
    *offset += iface_count * 2;
}

static void skip_fields(const uint8_t *data, uint32_t data_len, uint32_t *offset, uint16_t fields_count) {
    for (uint16_t i = 0; i < fields_count; i++) {
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

static void skip_methods(const uint8_t *data, uint32_t data_len, uint32_t *offset, uint16_t methods_count) {
    for (uint16_t i = 0; i < methods_count; i++) {
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

static void print_class_attributes(const ClassAttributeInfo *attrs, uint16_t count) {
    if (count == 0) {
        printf("\t (no attributes)\n");
        return;
    }

    for (uint16_t i = 0; i < count; i++) {
        const ClassAttributeInfo *a = &attrs[i];
        const char *attr_name = cp_get_utf8(NULL, a->attribute_name_index);
        printf("\t [%u] %s\n", i + 1, attr_name ? attr_name : "?");
        printf("\t\t Attribute Name: ConstantPoolInfo #%u <%s>\n",
               a->attribute_name_index, attr_name ? attr_name : "?");
        printf("\t\t Attribute Length: %u\n", a->attribute_length);

        if (a->tag == ATTR_CLASS_SOURCE_FILE) {
            printf("\t\t SourceFile: ConstantPoolInfo #%u <%s>\n",
                   a->info.source_file.sourcefile_index,
                   cp_get_utf8(NULL, a->info.source_file.sourcefile_index));
        } else if (a->tag == ATTR_CLASS_SIGNATURE) {
            printf("\t\t Signature: ConstantPoolInfo #%u <%s>\n",
                   a->info.signature.signature_index,
                   cp_get_utf8(NULL, a->info.signature.signature_index));
        } else if (a->tag == ATTR_CLASS_RUNTIME_VISIBLE_ANNOTATIONS ||
                   a->tag == ATTR_CLASS_RUNTIME_INVISIBLE_ANNOTATIONS) {
            printf("\t\t Number of annotations: %u\n",
                   a->info.annotations.num_annotations);
            printf("\t\t (raw bytes omitted)\n");
        } else if (a->tag == ATTR_CLASS_SYNTHETIC || a->tag == ATTR_CLASS_DEPRECATED) {
            printf("\t\t (attribute present, no payload)\n");
        } else {
            printf("\t\t (unknown attribute, %u bytes)\n", a->info.unknown.length);
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

    uint16_t minor_version = read_u2(data, &offset);
    uint16_t major_version = read_u2(data, &offset);
    parse_constant_pool(data, data_len, &offset);

    uint16_t class_access_flags = read_u2(data, &offset);
    uint16_t this_class = read_u2(data, &offset);
    uint16_t super_class = read_u2(data, &offset);
    uint16_t iface_count = read_u2(data, &offset);

    skip_interfaces(&offset, iface_count);

    uint16_t fields_count = read_u2(data, &offset);
    skip_fields(data, data_len, &offset, fields_count);

    uint16_t methods_count = read_u2(data, &offset);
    skip_methods(data, data_len, &offset, methods_count);

    uint16_t class_attr_count = read_u2(data, &offset);

    printf("General Information\n");
    printf("{\n");
    printf("\t Magic Number: \t\t\t 0x%08X\n", magic);
    printf("\t Minor Version: \t\t %u\n", minor_version);
    printf("\t Major Version: \t\t %u [%s]\n", major_version, major_version_to_java_string(major_version));
    printf("\t Constant Pool count: \t %u\n", g_cp_count);

    char class_flags_str[128];
    class_flags_to_string(class_access_flags, class_flags_str, sizeof(class_flags_str));
    printf("\t Access Flags: \t\t 0x%04X [%s]\n", class_access_flags, class_flags_str);
    printf("\t This Class: \t\t\t ConstantPoolInfo #%u <%s>\n", this_class, resolve_class_name(this_class));
    if (super_class == 0) {
        printf("\t Super class: \t\t\t none\n");
    } else {
        printf("\t Super class: \t\t\t ConstantPoolInfo #%u <%s>\n", super_class, resolve_class_name(super_class));
    }
    printf("\t Interfaces count: \t %u\n", iface_count);
    printf("\t Fields count: \t\t %u\n", fields_count);
    printf("\t Methods pool count: \t %u\n", methods_count);
    printf("\t Attributes pool count: %u\n", class_attr_count);
    printf("}\n\n");

    ClassAttributeInfo *attrs = parse_class_attributes(data, data_len, &offset, class_attr_count, NULL);
    if (!attrs) {
        fprintf(stderr, "parse_class_attributes retornou NULL\n");
        free_constant_pool_strings();
        free(data);
        return 1;
    }

    printf("Class Attributes\n");
    printf("{\n");
    print_class_attributes(attrs, class_attr_count);
    printf("}\n");

    free_class_attributes(attrs, class_attr_count);
    free_constant_pool_strings();
    free(data);
    return 0;
}
