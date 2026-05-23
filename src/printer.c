#include "printer.h"
#include "classfile.h"
#include "cp_resolver.h"
#include <math.h>
#include <string.h>

void print_fields(ClassFile *cf, FILE *out);
void print_methods(ClassFile *cf, FILE *out);
void print_class_attributes(ClassFile *cf, FILE *out);


static const char *major_to_java(u2 major)
{
    switch (major) {
        case 45: return "Java 1.1"; case 46: return "Java 1.2";
        case 47: return "Java 1.3"; case 48: return "Java 1.4";
        case 49: return "Java 5";   case 50: return "Java 6";
        case 51: return "Java 7";   case 52: return "Java 8";
        case 53: return "Java 9";   case 54: return "Java 10";
        case 55: return "Java 11";  case 56: return "Java 12";
        case 57: return "Java 13";  case 58: return "Java 14";
        case 59: return "Java 15";  case 60: return "Java 16";
        case 61: return "Java 17";  case 62: return "Java 18";
        case 63: return "Java 19";  case 64: return "Java 20";
        case 65: return "Java 21";  default: return "unknown";
    }
}

static void class_flags_to_string(u2 flags, char *buf, size_t buf_size)
{
    buf[0] = '\0';
    struct { u2 mask; const char *name; } table[] = {
        { 0x0001, "public"     }, { 0x0010, "final"      },
        { 0x0020, "super"      }, { 0x0200, "interface"  },
        { 0x0400, "abstract"   }, { 0x1000, "synthetic"  },
        { 0x2000, "annotation" }, { 0x4000, "enum"       },
        { 0x8000, "module"     },
    };
    for (size_t k = 0; k < sizeof(table)/sizeof(table[0]); k++) {
        if (flags & table[k].mask) {
            if (buf[0] != '\0') strncat(buf, " ", buf_size - strlen(buf) - 1);
            strncat(buf, table[k].name, buf_size - strlen(buf) - 1);
        }
    }
    if (buf[0] == '\0') strncpy(buf, "(none)", buf_size - 1);
}

static const char *cp_class_name(ClassFile *cf, u2 class_index)
{
    if (class_index == 0 || class_index >= cf->constant_pool_count) return "?";
    cp_info *cp = &cf->constant_pool[class_index];
    if (cp->tag != CONSTANT_Class) return "?";
    u2 ni = cp->info.class_info.name_index;
    if (ni >= cf->constant_pool_count) return "?";
    if (cf->constant_pool[ni].tag != CONSTANT_Utf8) return "?";
    return (char *)cf->constant_pool[ni].info.utf8_info.bytes;
}

void print_constant_pool_info(ClassFile *cf, u2 index, FILE *out)
{
    if (index == 0 || index >= cf->constant_pool_count) { fprintf(out, "?"); return; }
    cp_info *cp = &cf->constant_pool[index];
    switch (cp->tag) {
        case CONSTANT_Utf8:
            fprintf(out, "%s", cp->info.utf8_info.bytes); break;
        case CONSTANT_Class:
            print_constant_pool_info(cf, cp->info.class_info.name_index, out); break;
        case CONSTANT_String:
            print_constant_pool_info(cf, cp->info.string_info.string_index, out); break;
        case CONSTANT_NameAndType:
            print_constant_pool_info(cf, cp->info.name_and_type_info.name_index, out);
            fprintf(out, " ");
            print_constant_pool_info(cf, cp->info.name_and_type_info.descriptor_index, out);
            break;
        default:
            fprintf(out, "<unsupported>"); break;
    }
}

void print_class_file(ClassFile *cf, FILE *out)
{
    /* --- General Information --- */
    char flags_str[128];
    class_flags_to_string(cf->access_flags, flags_str, sizeof(flags_str));

    fprintf(out, "General Information\n{\n");
    fprintf(out, "\t Magic Number: \t\t\t 0x%08X\n", cf->magic);
    fprintf(out, "\t Minor Version: \t\t %d\n", cf->minor_version);
    fprintf(out, "\t Major Version: \t\t %d [%s]\n",
            cf->major_version, major_to_java(cf->major_version));
    fprintf(out, "\t Constant Pool count: \t\t %d\n", cf->constant_pool_count);
    fprintf(out, "\t Access Flags: \t\t\t 0x%04X [%s]\n",
            cf->access_flags, flags_str);
    fprintf(out, "\t This Class: \t\t\t ConstantPoolInfo #%d <%s>\n",
            cf->this_class, cp_class_name(cf, cf->this_class));
    if (cf->super_class == 0)
        fprintf(out, "\t Super class: \t\t\t none\n");
    else
        fprintf(out, "\t Super class: \t\t\t ConstantPoolInfo #%d <%s>\n",
                cf->super_class, cp_class_name(cf, cf->super_class));
    fprintf(out, "\t Interfaces count: \t\t %d\n", cf->interfaces_count);
    fprintf(out, "\t Fields count: \t\t\t %d\n", cf->fields_count);
    fprintf(out, "\t Methods pool count: \t\t %d\n", cf->methods_count);
    fprintf(out, "\t Attributes pool count: \t %d\n", cf->attributes_count);
    fprintf(out, "}\n\n");

    /* --- Constant Pool --- */
    fprintf(out, "Constant Pool (Member count: %d)\n{\n", cf->constant_pool_count);
    for (int i = 1; i < cf->constant_pool_count; i++) {
        cp_info *cp = &cf->constant_pool[i];
        switch (cp->tag) {
            case CONSTANT_Class:
                fprintf(out, "\t [%d] ConstClassInfo\n", i);
                fprintf(out, "\t\t Class name: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.class_info.name_index);
                print_constant_pool_info(cf, cp->info.class_info.name_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_Fieldref:
                fprintf(out, "\t [%d] ConstFieldRefInfo\n", i);
                fprintf(out, "\t\t Class name: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.fieldref_info.class_index);
                print_constant_pool_info(cf, cp->info.fieldref_info.class_index, out);
                fprintf(out, ">\n");
                fprintf(out, "\t\t Name and type: \t\t ConstantPoolInfo #%d <",
                        cp->info.fieldref_info.name_and_type_index);
                print_constant_pool_info(cf, cp->info.fieldref_info.name_and_type_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_Methodref:
                fprintf(out, "\t [%d] ConstMethodRefInfo\n", i);
                fprintf(out, "\t\t Class name: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.methodref_info.class_index);
                print_constant_pool_info(cf, cp->info.methodref_info.class_index, out);
                fprintf(out, ">\n");
                fprintf(out, "\t\t Name and type: \t\t ConstantPoolInfo #%d <",
                        cp->info.methodref_info.name_and_type_index);
                print_constant_pool_info(cf, cp->info.methodref_info.name_and_type_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_InterfaceMethodref:
                fprintf(out, "\t [%d] ConstInterfaceMethodRefInfo\n", i);
                fprintf(out, "\t\t Class name: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.interface_methodref_info.class_index);
                print_constant_pool_info(cf, cp->info.interface_methodref_info.class_index, out);
                fprintf(out, ">\n");
                fprintf(out, "\t\t Name and type: \t\t ConstantPoolInfo #%d <",
                        cp->info.interface_methodref_info.name_and_type_index);
                print_constant_pool_info(cf, cp->info.interface_methodref_info.name_and_type_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_String:
                fprintf(out, "\t [%d] ConstStrInfo\n", i);
                fprintf(out, "\t\t String: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.string_info.string_index);
                print_constant_pool_info(cf, cp->info.string_info.string_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_Integer:
                fprintf(out, "\t [%d] ConstIntInfo\n", i);
                fprintf(out, "\t\t Bytes: \t\t\t 0x%08X\n", cp->info.integer_info.bytes);
                fprintf(out, "\t\t Integer: \t\t\t %d\n",
                        (int32_t)cp->info.integer_info.bytes); break;
            case CONSTANT_Float: {
                fprintf(out, "\t [%d] ConstFloatInfo\n", i);
                fprintf(out, "\t\t Bytes: \t\t\t 0x%08X\n", cp->info.float_info.bytes);
                u4 b = cp->info.float_info.bytes;
                int32_t s = ((b >> 31) == 0) ? 1 : -1;
                int32_t e = (b >> 23) & 0xff;
                int32_t m = (e == 0) ? (b & 0x7fffff) << 1 : (b & 0x7fffff) | 0x800000;
                float f = (float)(s * m * pow(2, e - 150));
                fprintf(out, "\t\t Float: \t\t\t %f\n", f); break;
            }
            case CONSTANT_Long: {
                fprintf(out, "\t [%d] ConstLongInfo\n", i);
                fprintf(out, "\t\t High Bytes: \t\t\t 0x%08X\n", cp->info.long_info.high_bytes);
                fprintf(out, "\t\t Low Bytes: \t\t\t 0x%08X\n", cp->info.long_info.low_bytes);
                int64_t l = ((int64_t)cp->info.long_info.high_bytes << 32) +
                             cp->info.long_info.low_bytes;
                fprintf(out, "\t\t Long: \t\t\t\t %lld\n", (long long)l); break;
            }
            case CONSTANT_Double: {
                fprintf(out, "\t [%d] ConstDoubleInfo\n", i);
                fprintf(out, "\t\t High Bytes: \t\t\t 0x%08X\n", cp->info.double_info.high_bytes);
                fprintf(out, "\t\t Low Bytes: \t\t\t 0x%08X\n", cp->info.double_info.low_bytes);
                int64_t db = ((int64_t)cp->info.double_info.high_bytes << 32) +
                              cp->info.double_info.low_bytes;
                int32_t ds = ((db >> 63) == 0) ? 1 : -1;
                int32_t de = (int32_t)((db >> 52) & 0x7ffL);
                int64_t dm = (de == 0) ? (db & 0xfffffffffffffL) << 1
                                       : (db & 0xfffffffffffffL) | 0x10000000000000L;
                double d = ds * dm * pow(2, de - 1075);
                fprintf(out, "\t\t Double: \t\t\t %f\n", d); break;
            }
            case 0:
                fprintf(out, "\t [%d] (large numeric continued)\n", i); break;
            case CONSTANT_NameAndType:
                fprintf(out, "\t [%d] ConstNameTypeInfo\n", i);
                fprintf(out, "\t\t Name: \t\t\t\t ConstantPoolInfo #%d <",
                        cp->info.name_and_type_info.name_index);
                print_constant_pool_info(cf, cp->info.name_and_type_info.name_index, out);
                fprintf(out, ">\n");
                fprintf(out, "\t\t Descriptor: \t\t\t ConstantPoolInfo #%d <",
                        cp->info.name_and_type_info.descriptor_index);
                print_constant_pool_info(cf, cp->info.name_and_type_info.descriptor_index, out);
                fprintf(out, ">\n"); break;
            case CONSTANT_Utf8:
                fprintf(out, "\t [%d] ConstUtf8Info\n", i);
                fprintf(out, "\t\t Length of byte array: \t\t %d\n", cp->info.utf8_info.length);
                fprintf(out, "\t\t Length of string: \t\t %d\n", cp->info.utf8_info.length);
                fprintf(out, "\t\t String: \t\t\t %s\n", cp->info.utf8_info.bytes); break;
        }
        fprintf(out, "\n");
    }
    fprintf(out, "}\n\n");

    print_fields(cf, out);
    print_methods(cf, out);
    print_class_attributes(cf, out);
}
