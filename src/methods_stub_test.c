#include <stdio.h>
#include <stdlib.h>
#include "fields.h"
#include "methods.h"

/* Stub temporário — remover quando o M1 entregar constant_pool.c */
const char *cp_get_utf8(const ConstantPoolEntry *cp, uint16_t index)
{
    (void)cp; (void)index;
    return "???";
}

static void skip_constant_pool(const uint8_t *data, uint32_t data_len,
                                uint32_t *offset)
{
    if (*offset + 2 > data_len) return;
    uint16_t cp_count = (uint16_t)((data[*offset] << 8) | data[*offset + 1]);
    *offset += 2;

    for (uint16_t i = 1; i < cp_count; i++) {
        if (*offset >= data_len) break;
        uint8_t tag = data[(*offset)++];

        if      (tag == 1)  { uint16_t l = (uint16_t)((data[*offset]<<8)|data[*offset+1]); *offset += 2+l; }
        else if (tag == 3 || tag == 4)              { *offset += 4; }
        else if (tag == 5 || tag == 6)              { *offset += 8; i++; }
        else if (tag == 7 || tag == 8 || tag == 16
              || tag == 19 || tag == 20)            { *offset += 2; }
        else if (tag == 9 || tag == 10
              || tag == 11 || tag == 12)            { *offset += 4; }
        else if (tag == 15)                         { *offset += 3; }
        else if (tag == 17 || tag == 18)            { *offset += 4; }
        else { fprintf(stderr, "[stub] Tag desconhecida %u\n", tag); break; }
    }
}

static void skip_fields(const uint8_t *data, uint32_t data_len, uint32_t *offset)
{
    uint16_t count = (uint16_t)((data[*offset]<<8)|data[*offset+1]);
    *offset += 2;
    for (uint16_t i = 0; i < count; i++) {
        *offset += 6; /* access_flags + name_index + descriptor_index */
        uint16_t ac = (uint16_t)((data[*offset]<<8)|data[*offset+1]);
        *offset += 2;
        for (uint16_t j = 0; j < ac; j++) {
            *offset += 2; /* attribute_name_index */
            uint32_t al = ((uint32_t)data[*offset]<<24)|((uint32_t)data[*offset+1]<<16)
                         |((uint32_t)data[*offset+2]<<8)|data[*offset+3];
            *offset += 4 + al;
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

    uint32_t offset = 8;                         /* pula magic+minor+major */
    skip_constant_pool(data, len, &offset);       /* pula pool */
    offset += 6;                                  /* pula access_flags+this+super */
    uint16_t ifaces = (uint16_t)((data[offset]<<8)|data[offset+1]);
    offset += 2 + ifaces * 2;                    /* pula interfaces */
    skip_fields(data, len, &offset);              /* pula fields */

    uint16_t methods_count = (uint16_t)((data[offset]<<8)|data[offset+1]);
    offset += 2;

    printf("methods_count lido: %u\n\n", methods_count);

    MethodInfo *methods = parse_methods(data, len, &offset, methods_count, NULL);
    print_methods(methods, methods_count, NULL);
    free_methods(methods, methods_count);

    free(data);
    return 0;
}