#ifndef TIMERMGR_H
#define TIMERMGR_H

#include <Arduino.h>
#include <Ticker.h>
#include "DisplayMgr.h"
#include "AlarmMgr.h" // Para usar el sonido de alarma cuando el tiempo expire

// Estados internos del Temporizador
enum TimerState {
    TMR_CONFIG = 0, // Esperando que el usuario fije el tiempo inicial
    TMR_STOPPED,    // Tiempo inicial fijado, esperando START
    TMR_RUNNING,    // Cuenta regresiva activa
    TMR_PAUSED,     // Cuenta regresiva detenida
    TMR_EXPIRED     // Llegó a 00:00:00
};

class TimerMgr {
public:
    static void init();

    // --- Funciones de control (Callbacks) ---
    static void start();
    static void pause();
    static void reset(); // Devuelve al tiempo fijado inicialmente (fixedTime)
    
    // Función principal para dibujar y gestionar el estado (Llamar en loop)
    static void update();

    // --- Funciones de Configuración (Desde el Menú) ---
    // Setea el tiempo inicial (en segundos)
    static void setInitialTime(unsigned long seconds); 
    
    // Devuelve el estado actual
    static TimerState getState();
    static String getFormattedTime();

private:
    static void onTick();
    
    // Variables internas
    static Ticker ticker;
    // Tiempo total a contar (en centésimas de segundo)
    static unsigned long fixedTimeCentis; 
    // Tiempo restante (en centésimas de segundo)
    static volatile unsigned long remainingCentis; 
    static TimerState state;
    static bool dirty; 
};

#endif