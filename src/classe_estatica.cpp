#include "classe_estatica.hpp"
#include "exibidor_classe.hpp"  
#include <cstdlib>
#include <iostream>

using std::string;

static Valor valor_zerado_para_descritor(char tipo_descritor)
{
    switch (tipo_descritor) {
    case 'B': return faz_valor_byte(0);     
    case 'C': return faz_valor_char(0);     
    case 'D': return faz_valor_double(0);   
    case 'F': return faz_valor_float(0);    
    case 'I': return faz_valor_int(0);      
    case 'J': return faz_valor_long(0);     
    case 'S': return faz_valor_short(0);    
    case 'Z': return faz_valor_booleano(false); 
    default:  return faz_valor_referencia(0); 
    }
}

ClasseEstatica::ClasseEstatica(ArquivoClasse* arquivo_classe)
{
    this->arquivo_classe = arquivo_classe;

    for (int i = 0; i < arquivo_classe->fields_count; i++) {
        FieldInfo finfo = arquivo_classe->fields[i];

        if (finfo.access_flags & FIELD_FLAG_ACC_STATIC) {
            string nome_campo =
                formatar_constante(arquivo_classe->constant_pool, finfo.name_index);
            string descritor_campo =
                formatar_constante(arquivo_classe->constant_pool, finfo.descriptor_index);

            Valor valor = valor_zerado_para_descritor(descritor_campo[0]);
            insert_value(valor, nome_campo);
        }
    }
}

ArquivoClasse* ClasseEstatica::get_arquivo_classe()
{
    return this->arquivo_classe;
}

void ClasseEstatica::insert_value(const Valor& valor, const string& nome_campo)
{
    campos_estaticos[nome_campo] = valor;
}

Valor ClasseEstatica::get_value(const string& nome_campo)
{
    auto it = campos_estaticos.find(nome_campo);

    if (it == campos_estaticos.end()) {
        std::cerr << "NoSuchFieldError: " << nome_campo << std::endl;
        exit(1);
    }

    return it->second;
}

bool ClasseEstatica::check_field(const string& nome_campo)
{
    return campos_estaticos.count(nome_campo) > 0;
}
