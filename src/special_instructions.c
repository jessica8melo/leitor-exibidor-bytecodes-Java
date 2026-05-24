#include "bytecode.h"

char* getArrayType(u1 t){

    switch(t){

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
