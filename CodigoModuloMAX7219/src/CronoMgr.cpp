#include "CronoMgr.h"
#include "GlobalSettings.h" // Para saber la precisión

// Inicialización de variables estáticas
Ticker CronoMgr::ticker;
volatile unsigned long CronoMgr::centiseconds = 0;
CronoState CronoMgr::state = STOPPED;
bool CronoMgr::dirty = true;

// Función de interrupción (¡Debe ser rápida!)
void CronoMgr::onTick() {
    centiseconds++;
    dirty = true; // Avisamos al loop que hay nuevos datos para dibujar
}

void CronoMgr::init() {
    state = STOPPED;
    centiseconds = 0;
    // No arrancamos el ticker aquí, solo al dar START
}

void CronoMgr::start() {
    if (state != RUNNING) {
        // Ticker a 10ms (0.01s) para contar centésimas
        ticker.attach(0.01, onTick);
        state = RUNNING;
        
        // Actualizamos LCD una vez
        DisplayMgr::printLCD(0, "CRONOMETRO");
        DisplayMgr::printLCD(1, "OK:Pausa, DN:Rst");
    }
}

void CronoMgr::pause() {
    if (state == RUNNING) {
        ticker.detach();
        state = PAUSED;
        DisplayMgr::printLCD(1, "PAUSADO, OK:Cont");
    }
}

void CronoMgr::reset() {
    ticker.detach();
    state = STOPPED;
    centiseconds = 0;
    dirty = true; // Forzar repintado a 0
    DisplayMgr::printLCD(1, "STOP, OK:Iniciar");
}

CronoState CronoMgr::getState() {
    return state;
}

String CronoMgr::getFormattedTime() {
    unsigned long total_cs = centiseconds;
    unsigned long s_total = total_cs / 100;
    
    int cs = total_cs % 100;
    int s = s_total % 60;
    int m = s_total / 60;
    
    char buffer[12]; // Un poco más grande por si acaso

    if (GlobalSettings::cronoPrecision == PRECISION_CENTIS) {
        sprintf(buffer, "%02d:%02d:%02d", m, s, cs);
    } else { // PRECISION_SECONDS
        // Formato MM.SS.
        // La matriz de 8x8 no puede mostrar bien los dos puntos y que se vea claro
        // Usamos un punto para separar y dejamos espacio.
        sprintf(buffer, " %02d.%02d", m, s);
    }
    return String(buffer);
}

void CronoMgr::update() {
    // Solo dibujamos si hay cambios (dirty)
    if (dirty) {
        String timeStr = getFormattedTime();
        DisplayMgr::printMatrix(timeStr);
        dirty = false; // Ya dibujamos, esperamos el siguiente tick
    }
}