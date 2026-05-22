#include <stdio.h>
#include <stdlib.h>
#include "fields.h"

/* Stub temporário — remover quando o M1 entregar constant_pool.c */
const char *cp_get_utf8(const ConstantPoolEntry *cp, uint16_t index)
{
    (void)cp; (void)index;
    return "???";
}

/*
 * Pula o constant pool sem interpretá-lo.
 * Quando o M1 entregar o parser real, esta função some
 * e o offset vem pronto após cp_parse().
 */
static void skip_constant_pool(const uint8_t *data, uint32_t data_len,
                                uint32_t *offset)
{
    if (*offset + 2 > data_len) return;
    uint16_t cp_count = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
    *offset += 2;

    for (uint16_t i = 1; i < cp_count; i++) {
        if (*offset >= data_len) break;
        uint8_t tag = data[(*offset)++];

        if      (tag == 1)  { uint16_t len = (uint16_t)((data[*offset] << 8) | data[*offset+1]); *offset += 2 + len; }
        else if (tag == 3 || tag == 4)  { *offset += 4; }
        else if (tag == 5 || tag == 6)  { *offset += 8; i++; } /* Long/Double: 2 slots */
        else if (tag == 7 || tag == 8 || tag == 16 || tag == 19 || tag == 20) { *offset += 2; }
        else if (tag == 9 || tag == 10 || tag == 11 || tag == 12) { *offset += 4; }
        else if (tag == 15) { *offset += 3; }
        else if (tag == 17 || tag == 18) { *offset += 4; }
        else {
            fprintf(stderr, "[stub] Tag desconhecida %u no índice %u, offset %u\n",
                    tag, i, *offset);
            break;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.class>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror("fopen"); return 1; }

    fseek(f, 0, SEEK_END);
    uint32_t len = (uint32_t)ftell(f);
    rewind(f);

    uint8_t *data = malloc(len);
    if (!data) { perror("malloc"); fclose(f); return 1; }
    fread(data, 1, len, f);
    fclose(f);

    /* Pula: magic(4) + minor(2) + major(2) = 8 bytes */
    uint32_t offset = 8;

    /* Pula o constant pool inteiro */
    skip_constant_pool(data, len, &offset);

    /* Pula: access_flags(2) + this_class(2) + super_class(2) = 6 bytes */
    offset += 6;

    /* Pula interfaces: interfaces_count(2) + count * 2 bytes */
    uint16_t ifaces_count = (uint16_t)((data[offset] << 8) | data[offset + 1]);
    offset += 2 + ifaces_count * 2;

    /* Agora offset aponta para fields_count */
    uint16_t fields_count = (uint16_t)((data[offset] << 8) | data[offset + 1]);
    offset += 2;

    printf("fields_count lido: %u\n\n", fields_count);

    FieldInfo *fields = parse_fields(data, len, &offset, fields_count, NULL);
    print_fields(fields, fields_count, NULL);
    free_fields(fields, fields_count);

    free(data);
    return 0;
}