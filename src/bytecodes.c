#include "bytecode.h"


Opcode opcodeTable[256]={

/* constantes */

[0x00]={"nop",0},

[0x03]={"iconst_0",0},
[0x04]={"iconst_1",0},
[0x05]={"iconst_2",0},
[0x06]={"iconst_3",0},
[0x07]={"iconst_4",0},
[0x08]={"iconst_5",0},

[0x10]={"bipush",1},
[0x11]={"sipush",2},

[0x14]={"ldc2_w",2},

/* loads */

[0x18]={"dload",1},

[0x26]={"dload_0",0},
[0x27]={"dload_1",0},
[0x28]={"dload_2",0},
[0x29]={"dload_3",0},

[0x19]={"aload",1},

[0x2A]={"aload_0",0},
[0x2B]={"aload_1",0},
[0x2C]={"aload_2",0},
[0x2D]={"aload_3",0},


/* stores */

[0x36]={"istore",1},

[0x3B]={"istore_0",0},
[0x3C]={"istore_1",0},
[0x3D]={"istore_2",0},
[0x3E]={"istore_3",0},

[0x39]={"dstore",1},

[0x47]={"dstore_0",0},
[0x48]={"dstore_1",0},
[0x49]={"dstore_2",0},
[0x4A]={"dstore_3",0},

[0x4B]={"astore_0",0},
[0x4C]={"astore_1",0},
[0x4D]={"astore_2",0},
[0x4E]={"astore_3",0},


/* métodos */

[0xB7]={"invokespecial",2},

[0xB1]={"return",0},


/* operadores lógicos */

[0x7E]={"iand",0},
[0x7F]={"land",0},

[0x80]={"ior",0},
[0x81]={"lor",0},

[0x82]={"ixor",0},
[0x83]={"lxor",0},


/* casts */

[0x85]={"i2l",0},
[0x86]={"i2f",0},
[0x87]={"i2d",0},

[0x88]={"l2i",0},
[0x89]={"l2f",0},
[0x8A]={"l2d",0},

[0x8B]={"f2i",0},
[0x8C]={"f2l",0},
[0x8D]={"f2d",0},

[0x8E]={"d2i",0},
[0x8F]={"d2l",0},
[0x90]={"d2f",0},


/* arrays */

[0xBC]={"newarray",1},
[0xC5]={"multianewarray",3},


/* especiais */

[0xAA]={"tableswitch",-1},
[0xAB]={"lookupswitch",-1}

};



char* getArrayType(u1 type){

    switch(type){

        case 4:
            return "boolean";

        case 5:
            return "char";

        case 6:
            return "float";

        case 7:
            return "double";

        case 8:
            return "byte";

        case 9:
            return "short";

        case 10:
            return "int";

        case 11:
            return "long";

        default:
            return "unknown";
    }

}



void print_bytecodes(
ClassFile *cf,
u1 *code,
u4 code_length,
FILE *out
){

    u4 pc=0;

    while(pc<code_length){

        u4 current_pc=pc;

        u1 opcode=code[pc];

        char *name=
        opcodeTable[opcode].mnemonic;

        if(name==NULL)
            name="unknown";

        fprintf(
        out,
        "\t\t\t\t %d: %s",
        current_pc,
        name
        );

        pc++;

        int operands=
        opcodeTable[opcode].operands;


        /* newarray */

        if(opcode==0xBC){

            fprintf(
            out,
            " %s",
            getArrayType(code[pc])
            );

            pc++;

        }


        /* multianewarray */

        else if(opcode==0xC5){

            u2 index=
            (code[pc]<<8)
            | code[pc+1];

            pc+=2;

            u1 dimensions=
            code[pc];

            pc++;

            fprintf(
            out,
            " #%d dim:%d",
            index,
            dimensions
            );

        }

        else{

            for(
            int i=0;
            i<operands;
            i++
            ){

                fprintf(
                out,
                " %d",
                code[pc]
                );

                pc++;

            }

        }

        fprintf(out,"\n");

    }

}