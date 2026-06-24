#ifndef FRAME_HPP
#define FRAME_HPP

#include "tipos_runtime.hpp"
#include "classe_estatica.hpp"
#include "classe_instancia.hpp"
#include <map>
#include <stack>
#include <string>
#include <vector>

class Frame {
public:

    Frame(ClasseInstancia* objeto, ClasseEstatica* classe_runtime,
          std::string nome_metodo, std::string descritor_metodo,
          std::vector<Valor> argumentos);

    Frame(ClasseEstatica* classe_runtime, std::string nome_metodo,
          std::string descritor_metodo,
          std::vector<Valor> argumentos = std::vector<Valor>());

    ~Frame();

    ConstantPoolInfo** get_constant_pool();

    Valor get_local_variable_value(uint32_t index);

    void set_local_variable(Valor valor, uint32_t index);

    void push_operand_stack(Valor operando);

    Valor pop_operand_stack();

    std::stack<Valor> copy_operand_stack();

    void load_operand_stack(std::stack<Valor> backup);

    ClasseInstancia* get_object();

    u1* get_code(uint32_t address);

    u4 pc;

    u2 get_local_variables_vector_size();

    u4 get_code_size();

private:

    MethodInfo* obterMethodNamed(ClasseEstatica* classe_runtime,
                                 const std::string& nome,
                                 const std::string& descritor);

    void encontrarAttributes();

    ClasseEstatica* class_runtime;                 
    ClasseInstancia* object;                       
    MethodInfo method;                             
    CodeAttribute* code_attribute;                 
    ExceptionsAttribute* exceptions_attribute;     
    std::map<uint32_t, Valor> local_variables;     
    std::stack<Valor> operand_stack;               
};

#endif 
