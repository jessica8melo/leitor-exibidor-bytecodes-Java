#ifndef PILHA_EXECUCAO_HPP
#define PILHA_EXECUCAO_HPP

#include "frame.hpp"
#include <cstdint>
#include <stack>

class PilhaExecucao {
public:

    static PilhaExecucao& instancia()
    {
        static PilhaExecucao unica;
        return unica;
    }

    ~PilhaExecucao();

    void empilhar_frame(Frame* frame);

    Frame* frame_topo();

    bool desempilhar_frame();

    uint32_t tamanho();

    PilhaExecucao(const PilhaExecucao&)            = delete;
    PilhaExecucao& operator=(const PilhaExecucao&) = delete;

private:
    PilhaExecucao() = default;

    std::stack<Frame*> frames;
};

#endif 
