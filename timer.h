#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    bool running;
    
public:
    Timer();
    
    // Iniciar el timer
    void start();
    
    // Detener el timer
    void stop();
    
    // Obtener tiempo transcurrido en milisegundos
    double getElapsedMilliseconds() const;
    
    // Obtener tiempo transcurrido en microsegundos
    double getElapsedMicroseconds() const;
    
    // Obtener tiempo transcurrido en segundos
    double getElapsedSeconds() const;
    
    // Imprimir tiempo transcurrido
    void printElapsed(const char* label = "Tiempo transcurrido") const;
    
    // Resetear el timer
    void reset();
    
    // Verificar si está corriendo
    bool isRunning() const;
    
    // Métodos estáticos para medición rápida
    static double measureFunction(void (*func)());
    
    template<typename Func>
    static double measureFunction(Func func) {
        Timer timer;
        timer.start();
        func();
        timer.stop();
        return timer.getElapsedMilliseconds();
    }
};

#endif