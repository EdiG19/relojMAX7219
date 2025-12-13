#ifndef CRONOMGR_H
#define CRONOMGR_H

#include <Arduino.h>
#include <Ticker.h>
#include "DisplayMgr.h" // Necesitamos acceso a las pantallas

// Estados internos del Cronómetro
enum CronoState {
    STOPPED,
    RUNNING,
    PAUSED
};

class CronoMgr {
public:
    // Configura el Ticker
    static void init();
    
    // Funciones de control (Callbacks)
    static void start();
    static void pause();
    static void reset();
    
    // Función principal para manejar la lógica visual (Llamar en el loop cuando estemos en modo Crono)
    static void update();
    
    // Devuelve el estado actual (para saber qué pintar en el menú)
    static CronoState getState();

private:
    // Función llamada por el Ticker (Interrupción)
    static void onTick();
    
    // Convierte el tiempo a formato texto
    static String getFormattedTime();

    // Variables internas
    static Ticker ticker;
    static volatile unsigned long centiseconds; // Cuenta centésimas
    static CronoState state;
    static bool dirty; // Bandera para saber si hay que redibujar pantalla
};

#endif