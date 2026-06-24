#include "pilha_execucao.hpp"

PilhaExecucao::~PilhaExecucao()
{
    while (!frames.empty()) {
        delete frames.top();
        frames.pop();
    }
}

void PilhaExecucao::empilhar_frame(Frame* frame)
{
    frames.push(frame);
}

Frame* PilhaExecucao::frame_topo()
{
    if (frames.size() == 0) {
        return NULL;
    }
    return frames.top();
}

bool PilhaExecucao::desempilhar_frame()
{
    if (frames.size() == 0) {
        return false;
    }

    Frame* frame = frames.top();
    frames.pop();
    delete frame;
    return true;
}

uint32_t PilhaExecucao::tamanho()
{
    return frames.size();
}
