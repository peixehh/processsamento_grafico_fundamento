#include "check.h"
#include "Timer.h"

#include <chrono>
#include <thread>

int main()
{
    Timer t;
    // espera para fechar um frame de 60 FPS (16,67 ms) depois de gastar 5 ms
    CHECK_NEAR(t.calcWaitingTime(60, 5.0), 1000.0 / 60.0 - 5.0, 1e-9);
    CHECK(t.calcWaitingTime(60, 20.0) < 0.0);             // frame estourou nao espera
    CHECK_NEAR(t.calcWaitingTime(30, 0.0), 1000.0 / 30.0, 1e-9);

    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    t.finish();
    CHECK(t.getElapsedTimeMs() >= 15.0);                  // dormiu 20 ms (folga para o relogio)
    CHECK(t.getElapsedTimeMs() < 1000.0);
    CHECK_NEAR(t.getElapsedTime() * 1000.0, t.getElapsedTimeMs(), 1e-6);
    return resultado();
}
