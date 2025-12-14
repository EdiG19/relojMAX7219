#ifndef TIMERMGR_H
#define TIMERMGR_H

#include <Ticker.h>
#include "DisplayMgr.h"
#include "AlarmMgr.h"

// Enum para la precisión del temporizador
enum TimerPrecision {
    PRECISION_HMS,      // Horas:Minutos:Segundos
    PRECISION_MS_CS     // Minutos:Segundos:Centésimas
};

// Enum para el estado del temporizador
enum TimerState {
    TMR_STOPPED,
    TMR_RUNNING,
    TMR_PAUSED,
    TMR_EXPIRED
};

class TimerMgr {
public:
    TimerMgr();
    void init();

    // Configuración
    void setPrecision(TimerPrecision precision);
    void setInitialTime(unsigned long seconds);

    // Controles
    void start();
    void pause();
    void reset();

    // Actualización y estado
    void update();
    TimerState getState();
    String getFormattedTime();
    TimerPrecision getPrecision();

private:
    static void tick_wrapper(); // El wrapper que Ticker llamará
    void tick(); // El método de instancia real

    TimerPrecision currentPrecision;
    TimerState state;

    Ticker timerTicker;
    unsigned long initialTime_cs; // Tiempo inicial en centésimas de segundo
    volatile unsigned long remainingTime_cs; // Tiempo restante en centésimas de segundo

    volatile bool dirty; // Para saber si hay que redibujar

    static TimerMgr* instance; // Puntero a la instancia para el wrapper estático
};

#endif // TIMERMGR_H
