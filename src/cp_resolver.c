#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../include/cp_resolver.h"



char *get_utf8(ClassFile *cf, uint16_t index)
{
    cp_info *cp = cf->constant_pool;

    if(index==0 ||
       index>=cf->constant_pool_count)
        return NULL;

    if(cp[index].tag!=CONSTANT_Utf8)
        return NULL;

    uint16_t len=
    cp[index].info.utf8_info.length;

    char *str=
    malloc(len+1);

    memcpy(
        str,
        cp[index]
        .info
        .utf8_info
        .bytes,
        len
    );

    str[len]='\0';

    return str;
}




char* resolve_cp_for_bytecode(
ClassFile *cf,
u2 index
){

    static char buffer[512];

    if(index==0 ||
       index>=cf->constant_pool_count){

        sprintf(
        buffer,
        "invalid"
        );

        return buffer;
    }

    cp_info *cp=
    &cf->constant_pool[index];



    switch(cp->tag){


        case CONSTANT_Class:{

            char *name=
            get_utf8(
            cf,
            cp->info.class_info.name_index
            );

            sprintf(
            buffer,
            "%s",
            name
            );

            free(name);

            break;
        }


        case CONSTANT_NameAndType:{

            char *name=
            get_utf8(
            cf,
            cp->info.name_and_type_info.name_index
            );

            char *desc=
            get_utf8(
            cf,
            cp->info.name_and_type_info.descriptor_index
            );


            sprintf(
            buffer,
            "%s %s",
            name,
            desc
            );

            free(name);
            free(desc);

            break;
        }



        case CONSTANT_Methodref:{

            u2 classIndex=
            cp->info.methodref_info.class_index;

            u2 ntIndex=
            cp->info.methodref_info
            .name_and_type_index;

            char classStr[256];
            char ntStr[256];

            strcpy(
            classStr,
            resolve_cp_for_bytecode(
            cf,
            classIndex
            ));

            strcpy(
            ntStr,
            resolve_cp_for_bytecode(
            cf,
            ntIndex
            ));


            sprintf(
            buffer,
            "%s.%s",
            classStr,
            ntStr
            );

            break;
        }



        case CONSTANT_Fieldref:{

            u2 classIndex=
            cp->info.fieldref_info.class_index;

            u2 ntIndex=
            cp->info.fieldref_info
            .name_and_type_index;

            char classStr[256];
            char ntStr[256];

            strcpy(
            classStr,
            resolve_cp_for_bytecode(
            cf,
            classIndex
            ));

            strcpy(
            ntStr,
            resolve_cp_for_bytecode(
            cf,
            ntIndex
            ));

            sprintf(
            buffer,
            "%s.%s",
            classStr,
            ntStr
            );

            break;
        }



        case CONSTANT_String:{

            char *str=
            get_utf8(
            cf,
            cp->info.string_info.string_index
            );

            sprintf(
            buffer,
            "%s",
            str
            );

            free(str);

            break;
        }



        case CONSTANT_Double:{

            uint64_t bits=
            (
            ((uint64_t)
            cp->info.double_info.high_bytes)
            <<32
            )
            |
            cp->info.double_info.low_bytes;

            double value;

            memcpy(
            &value,
            &bits,
            sizeof(double)
            );

            sprintf(
            buffer,
            "%lf",
            value
            );

            break;
        }



        case CONSTANT_Float:{

            uint32_t bits=
            cp->info.float_info.bytes;

            float value;

            memcpy(
            &value,
            &bits,
            sizeof(float)
            );

            sprintf(
            buffer,
            "%f",
            value
            );

            break;
        }



        case CONSTANT_Long:{

            int64_t value=
            (
            ((int64_t)
            cp->info.long_info.high_bytes)
            <<32
            )
            |
            cp->info.long_info.low_bytes;

            sprintf(
            buffer,
            "%lld",
            value
            );

            break;
        }



        case CONSTANT_Integer:{

            sprintf(
            buffer,
            "%d",
            cp->info.integer_info.bytes
            );

            break;
        }



        default:

            sprintf(
            buffer,
            "CP#%d",
            index
            );

    }

    return buffer;

}