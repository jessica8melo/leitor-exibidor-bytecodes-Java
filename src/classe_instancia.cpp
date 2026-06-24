#include "classe_instancia.hpp"
#include "exibidor_classe.hpp"  
#include <cstdlib>
#include <iostream>

using std::string;

static const u2 CLASS_FLAG_ACC_ABSTRACT = 0x0400;

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

ClasseInstancia::ClasseInstancia(ClasseEstatica* classe_runtime)
    : classe_runtime(classe_runtime)
{
    ArquivoClasse* arquivo_classe = classe_runtime->get_arquivo_classe();
    FieldInfo* fields = arquivo_classe->fields;

    if ((arquivo_classe->access_flags & CLASS_FLAG_ACC_ABSTRACT) != 0) {
        std::cerr << "InstantiationError" << std::endl;
        exit(1);
    }

    const u2 mascara_static_final = FIELD_FLAG_ACC_STATIC | FIELD_FLAG_ACC_FINAL;

    for (int i = 0; i < arquivo_classe->fields_count; i++) {
        FieldInfo field = fields[i];

        if ((field.access_flags & mascara_static_final) == 0) {
            string nome_campo =
                formatar_constante(arquivo_classe->constant_pool, field.name_index);
            string descritor_campo =
                formatar_constante(arquivo_classe->constant_pool, field.descriptor_index);

            Valor valor = valor_zerado_para_descritor(descritor_campo[0]);
            insert_value_into_field(valor, nome_campo);
        }
    }
}

ClasseInstancia::~ClasseInstancia()
{
}

TipoObjeto ClasseInstancia::tipo_objeto()
{
    return INSTANCIA_CLASSE;
}

ClasseEstatica* ClasseInstancia::get_classe_runtime()
{
    return classe_runtime;
}

void ClasseInstancia::insert_value_into_field(Valor valor, const string& nome_campo)
{
    campos_instancia[nome_campo] = valor;
}

Valor ClasseInstancia::get_value_from_field(const string& nome_campo)
{
    if (campos_instancia.count(nome_campo) == 0) {
        std::cerr << "NoSuchFieldError" << std::endl;
        exit(1);
    }

    return campos_instancia[nome_campo];
}

bool ClasseInstancia::field_exists(const string& nome_campo)
{
    return campos_instancia.count(nome_campo) > 0;
}
