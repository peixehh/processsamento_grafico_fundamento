#pragma once
#include <chrono>

// Timer da aula "Cenarios 2D camadas e animacao" mede o tempo de um quadro e calcula
// quanto falta esperar para fechar 1000/fps milissegundos.
// (steady_clock em vez de system_clock o relogio do sistema pode pular.)
class Timer
{
public:
    void start()  { begin_ = std::chrono::steady_clock::now(); }
    void finish() { end_ = std::chrono::steady_clock::now(); }

    double getElapsedTimeMs() const { return getElapsedTime() * 1000.0; }
    double getElapsedTime() const
    {
        std::chrono::duration<double> elapsed_seconds = end_ - begin_;
        return elapsed_seconds.count();
    }

    // Tempo de espera (ms) para um quadro de 1000/fps ms, descontando o que ja foi gasto.
    double calcWaitingTime(int fps, double elapsedMs) const
    {
        double wt = 1000.0 / static_cast<double>(fps) - elapsedMs;
        return wt;
    }

private:
    std::chrono::steady_clock::time_point begin_, end_;
};
