#include "bytecode.h"

Opcode opcodeTable[256]={

[0x2a]={"aload_0",0},
[0xb1]={"return",0},

[0xb7]={"invokespecial",2},
[0xb6]={"invokevirtual",2},
[0xb8]={"invokestatic",2},

[0x12]={"ldc",1},

[0x7e]={"iand",0},
[0x7f]={"land",0},
[0x80]={"ior",0},
[0x81]={"lor",0},
[0x82]={"ixor",0},
[0x83]={"lxor",0},

[0x85]={"i2l",0},
[0x86]={"i2f",0},
[0x87]={"i2d",0},

[0x88]={"l2i",0},
[0x89]={"l2f",0},
[0x8a]={"l2d",0},

[0x8b]={"f2i",0},
[0x8c]={"f2l",0},
[0x8d]={"f2d",0},

[0x8e]={"d2i",0},
[0x8f]={"d2l",0},
[0x90]={"d2f",0},

[0xbc]={"newarray",1},

[0xc5]={"multianewarray",3},

[0xaa]={"tableswitch",-1},

[0xab]={"lookupswitch",-1}

};
