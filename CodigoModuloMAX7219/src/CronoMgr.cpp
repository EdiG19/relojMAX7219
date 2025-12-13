#include "CronoMgr.h"

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
        DisplayMgr::printLCD(1, "[->]Pausa [||]Rst");
    }
}

void CronoMgr::pause() {
    if (state == RUNNING) {
        ticker.detach();
        state = PAUSED;
        DisplayMgr::printLCD(1, "PAUSADO [->]Cont");
    }
}

void CronoMgr::reset() {
    ticker.detach();
    state = STOPPED;
    centiseconds = 0;
    dirty = true; // Forzar repintado a 00:00:00
    DisplayMgr::printLCD(1, "STOP [->]Iniciar");
}

CronoState CronoMgr::getState() {
    return state;
}

String CronoMgr::getFormattedTime() {
    // Matemáticas para MM:SS:CS
    unsigned long total = centiseconds;
    int cs = total % 100;
    unsigned long s_total = total / 100;
    int s = s_total % 60;
    int m = s_total / 60;
    
    char buffer[10];
    sprintf(buffer, "%02d:%02d:%02d", m, s, cs);
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