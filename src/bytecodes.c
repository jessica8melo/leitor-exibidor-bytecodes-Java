#include "../include/bytecode.h"
#include "../include/cp_resolver.h"

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

[0x12]={"ldc",1},
[0x13]={"ldc_w",2},
[0x14]={"ldc2_w",2},


/* loads */

[0x15]={"iload",1},

[0x1A]={"iload_0",0},
[0x1B]={"iload_1",0},
[0x1C]={"iload_2",0},
[0x1D]={"iload_3",0},

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



/* lógicos */

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



/* desvios */

[0xA7]={"goto",2},



/* métodos */

[0xB6]={"invokevirtual",2},
[0xB7]={"invokespecial",2},
[0xB8]={"invokestatic",2},
[0xB9]={"invokeinterface",4},

[0xB2]={"getstatic",2},
[0xB3]={"putstatic",2},
[0xB4]={"getfield",2},
[0xB5]={"putfield",2},

[0xBB]={"new",2},

[0xC0]={"checkcast",2},
[0xC1]={"instanceof",2},

[0xB1]={"return",0},



/* arrays */

[0xBC]={"newarray",1},
[0xC5]={"multianewarray",3},



/* especiais */

[0xAA]={"tableswitch",-1},
[0xAB]={"lookupswitch",-1}

};



int readU4(u1 *code,int pos){

    return
    ((code[pos]&0xFF)<<24)
    |
    ((code[pos+1]&0xFF)<<16)
    |
    ((code[pos+2]&0xFF)<<8)
    |
    (code[pos+3]&0xFF);

}



char* getArrayType(u1 type){

    switch(type){

        case 4:return "boolean";
        case 5:return "char";
        case 6:return "float";
        case 7:return "double";
        case 8:return "byte";
        case 9:return "short";
        case 10:return "int";
        case 11:return "long";

        default:return "unknown";
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

    u1 opcode=
    code[pc];

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



    if(opcode==0xBC){

        fprintf(
        out,
        " %s",
        getArrayType(code[pc])
        );

        pc++;

    }



    else if(opcode==0xC5){

        u2 index=
        (code[pc]<<8)
        |code[pc+1];

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



    else if(opcode==0xAA){

        int padding=
        (4-((current_pc+1)%4))%4;

        pc+=padding;

        int defaultOffset=readU4(code,pc);
        pc+=4;

        int low=readU4(code,pc);
        pc+=4;

        int high=readU4(code,pc);
        pc+=4;

        fprintf(out,"\n\t\t\t\t {\n");

        fprintf(out,"\t\t\t\t low:%d\n",low);
        fprintf(out,"\t\t\t\t high:%d\n",high);
        fprintf(out,"\t\t\t\t default:%d\n",defaultOffset);

        int n=high-low+1;

        for(int i=0;i<n;i++){

            int jump=readU4(code,pc);
            pc+=4;

            fprintf(
            out,
            "\t\t\t\t %d -> %d\n",
            low+i,
            jump
            );

        }

        fprintf(out,"\t\t\t\t }");

    }



    else if(opcode==0xAB){

        int padding=
        (4-((current_pc+1)%4))%4;

        pc+=padding;

        int defaultOffset=
        readU4(code,pc);

        pc+=4;

        int npairs=
        readU4(code,pc);

        pc+=4;

        fprintf(out,"\n\t\t\t\t {\n");

        fprintf(out,"\t\t\t\t npairs:%d\n",npairs);

        fprintf(out,"\t\t\t\t default:%d\n",defaultOffset);

        for(int i=0;i<npairs;i++){

            int match=
            readU4(code,pc);

            pc+=4;

            int offset=
            readU4(code,pc);

            pc+=4;

            fprintf(
            out,
            "\t\t\t\t %d -> %d\n",
            match,
            offset
            );

        }

        fprintf(out,"\t\t\t\t }");

    }



    else{

        if(

            opcode==0xB6 ||
            opcode==0xB7 ||
            opcode==0xB8 ||
            opcode==0xB9 ||

            opcode==0xB2 ||
            opcode==0xB3 ||
            opcode==0xB4 ||
            opcode==0xB5 ||

            opcode==0xBB ||

            opcode==0xC0 ||
            opcode==0xC1 ||

            opcode==0x12 ||
            opcode==0x13 ||
            opcode==0x14
        ){

            u2 index;

            if(opcode==0x12){

                index=
                code[pc];

                pc++;

            }
            else{

                index=
                (code[pc]<<8)
                |
                code[pc+1];

                pc+=2;
            }

            fprintf(
            out,
            " #%d <%s>",
            index,
            resolve_cp_for_bytecode(
            cf,
            index
            )
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

    }

    fprintf(out,"\n");

}

}