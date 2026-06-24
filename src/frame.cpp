#include "frame.hpp"
#include "area_metodos.hpp"
#include "exibidor_classe.hpp"  
#include "util_classe.hpp"      
#include <cassert>
#include <cstdlib>
#include <iostream>

using std::string;
using std::vector;

Frame::Frame(ClasseInstancia* objeto, ClasseEstatica* classe_runtime,
             string nome_metodo, string descritor_metodo, vector<Valor> argumentos)
    : pc(0), object(objeto)
{

    for (int i = 0; i < (signed)argumentos.size(); i++) {
        local_variables[i] = argumentos[i];
    }

    MethodInfo* metodo = obterMethodNamed(classe_runtime, nome_metodo, descritor_metodo);
    assert(metodo != NULL);
    method = *metodo;

    assert((method.access_flags & FIELD_FLAG_ACC_STATIC) == 0);

    encontrarAttributes();
}

Frame::Frame(ClasseEstatica* classe_runtime, string nome_metodo,
             string descritor_metodo, vector<Valor> argumentos)
    : pc(0), object(NULL)
{

    for (int i = 0; i < (signed)argumentos.size(); i++) {
        local_variables[i] = argumentos[i];
    }

    MethodInfo* metodo = obterMethodNamed(classe_runtime, nome_metodo, descritor_metodo);
    assert(metodo != NULL);
    method = *metodo;

    assert((method.access_flags & FIELD_FLAG_ACC_STATIC) != 0);

    encontrarAttributes();
}

Frame::~Frame()
{
}

ConstantPoolInfo** Frame::get_constant_pool()
{
    return &(class_runtime->get_arquivo_classe()->constant_pool);
}

Valor Frame::get_local_variable_value(uint32_t index)
{
    if (index >= code_attribute->max_locals) {
        std::cerr << "Trying to get inexistent local variable" << std::endl;
        exit(1);
    }
    return local_variables[index];
}

void Frame::set_local_variable(Valor valor, uint32_t index)
{
    if (index >= code_attribute->max_locals) {
        std::cerr << "Trying to set inexistent local variable" << std::endl;
        exit(1);
    }
    local_variables[index] = valor;
}

void Frame::push_operand_stack(Valor operando)
{
    operand_stack.push(operando);
}

Valor Frame::pop_operand_stack()
{
    if (operand_stack.size() == 0) {
        std::cerr << "IndexOutOfBoundsException" << std::endl;
        exit(1);
    }

    Valor topo = operand_stack.top();
    operand_stack.pop();
    return topo;
}

std::stack<Valor> Frame::copy_operand_stack()
{
    return operand_stack;
}

void Frame::load_operand_stack(std::stack<Valor> backup)
{
    operand_stack = backup;
}

ClasseInstancia* Frame::get_object()
{
    return object;
}

u1* Frame::get_code(uint32_t address)
{
    return code_attribute->code + address;
}

MethodInfo* Frame::obterMethodNamed(ClasseEstatica* classe_runtime,
                                    const string& nome, const string& descritor)
{
    AreaMetodos& area_metodos = AreaMetodos::instancia();
    ClasseEstatica* classe_atual = classe_runtime;
    MethodInfo* metodo;

    while (classe_atual != NULL) {
        ArquivoClasse* arquivo = classe_atual->get_arquivo_classe();

        for (int i = 0; i < arquivo->methods_count; i++) {
            metodo = &(arquivo->methods[i]);
            string nome_metodo = formatar_constante(arquivo->constant_pool, metodo->name_index);
            string desc_metodo = formatar_constante(arquivo->constant_pool, metodo->descriptor_index);

            if (nome_metodo == nome && desc_metodo == descritor) {

                this->class_runtime = classe_atual;
                return metodo;
            }
        }

        if (arquivo->super_class == 0) {
            classe_atual = NULL;
        } else {
            string nome_super = formatar_constante(arquivo->constant_pool, arquivo->super_class);
            classe_atual = area_metodos.obter_classe(nome_super);
        }
    }

    return NULL;
}

void Frame::encontrarAttributes()
{
    ConstantPoolInfo* constant_pool = *get_constant_pool();
    code_attribute = NULL;
    exceptions_attribute = NULL;

    for (int i = 0; i < method.attributes_count; i++) {
        AttributeInfo* attr = &(method.attributes[i]);
        ConstUtf8Info nome_attr = constant_pool[attr->attribute_name_index - 1].info.utf8_info;

        if (UtilClasse::utf8_igual_a(nome_attr, "Code")) {
            code_attribute = &(attr->info.code_info);
            if (exceptions_attribute != NULL)
                break;
        } else if (UtilClasse::utf8_igual_a(nome_attr, "Exceptions")) {
            exceptions_attribute = &(attr->info.exceptions_info);
            if (code_attribute != NULL)
                break;
        }
    }
}

u2 Frame::get_local_variables_vector_size()
{
    return code_attribute->max_locals;
}

u4 Frame::get_code_size()
{
    return code_attribute->code_length;
}
