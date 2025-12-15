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
    static void init();

    // Configuración
    static void setPrecision(TimerPrecision precision);
    static void setInitialTime(unsigned long seconds);

    // Controles
    static void start();
    static void pause();
    static void reset();

    // Actualización y estado
    static void update();
    static TimerState getState();
    static String getFormattedTime();
    static TimerPrecision getPrecision();

private:
    static void onTick(); // El método de instancia real

    static TimerPrecision currentPrecision;
    
    static TimerState state;

    static Ticker timerTicker;
    static unsigned long initialTime_cs; // Tiempo inicial en centésimas de segundo
    static volatile unsigned long remainingTime_cs; // Tiempo restante en centésimas de segundo

    static volatile bool dirty; // Para saber si hay que redibujar

    static TimerMgr* instance; // Puntero a la instancia para el wrapper estático
};

#endif // TIMERMGR_H
