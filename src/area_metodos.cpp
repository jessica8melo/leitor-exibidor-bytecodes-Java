#include "area_metodos.hpp"
#include "leitor_classe.hpp"
#include "exibidor_classe.hpp"  
#include "executor.hpp"         
#include "frame.hpp"            
#include "pilha_execucao.hpp"   
#include <cstdio>
#include <cstdlib>
#include <iostream>

using std::string;

ClasseEstatica* AreaMetodos::carregar_classe(const string& nome)
{

    if (classes.count(nome) > 0) {
        return obter_classe(nome);
    }

    string caminho = nome;
    const string ext = ".class";
    if (caminho.size() < ext.size()
        || caminho.compare(caminho.size() - ext.size(), ext.size(), ext) != 0) {
        caminho = class_path + caminho + ext;
    }

    FILE* fp = fopen(caminho.c_str(), "rb");
    if (fp == NULL) {
        std::cerr << "No support for this class: " << nome << std::endl;
        exit(1);
    }

    ArquivoClasse* arquivo = LeitorClasse::instancia().ler_arquivo(fp);
    fclose(fp);

    ClasseEstatica* classe_runtime = new ClasseEstatica(arquivo);

    if (!inserir_classe(classe_runtime)) {
        string chave = formatar_constante(arquivo->constant_pool, arquivo->this_class);
        ClasseEstatica* existente = obter_classe(chave);
        delete classe_runtime;
        return existente;
    }

    Executor& executor = Executor::instancia();
    if (executor.verifica_metodo(classe_runtime, "<clinit>", "()V")) {
        PilhaExecucao::instancia().empilhar_frame(
            new Frame(classe_runtime, "<clinit>", "()V"));
    }

    return classe_runtime;
}

void AreaMetodos::registrar_classe(ClasseEstatica* c)
{
    inserir_classe(c);  
}

ClasseEstatica* AreaMetodos::obter_classe(const string& nome)
{
    auto it = classes.find(nome);
    if (it == classes.end()) {
        return nullptr;
    }
    return it->second;
}

bool AreaMetodos::inserir_classe(ClasseEstatica* classe_runtime)
{
    ArquivoClasse* arquivo = classe_runtime->get_arquivo_classe();
    string chave = formatar_constante(arquivo->constant_pool, arquivo->this_class);

    if (classes.count(chave) > 0) {
        return false;
    }

    classes[chave] = classe_runtime;
    return true;
}
