#include "TimerMgr.h"

// Inicialización de variables estáticas
TimerPrecision TimerMgr::currentPrecision = PRECISION_MS_CS;
TimerState TimerMgr::state = TMR_STOPPED;
Ticker TimerMgr::timerTicker;
unsigned long TimerMgr::initialTime_cs = 0;
volatile unsigned long TimerMgr::remainingTime_cs = 0;
volatile bool TimerMgr::dirty = true;
TimerMgr* TimerMgr::instance = nullptr;

TimerMgr::TimerMgr() {
    // Constructor privado para el patrón singleton (aunque usemos estáticos)
}

void TimerMgr::init() {
    state = TMR_STOPPED;
    remainingTime_cs = initialTime_cs;
    dirty = true;
}

void TimerMgr::setPrecision(TimerPrecision precision) {
    currentPrecision = precision;
    dirty = true; // Forzar redibujado con el nuevo formato
}

void TimerMgr::setInitialTime(unsigned long seconds) {
    // Siempre almacenamos en centésimas de segundo
    initialTime_cs = seconds * 100;
    if (state == TMR_STOPPED || state == TMR_EXPIRED) {
        remainingTime_cs = initialTime_cs;
        dirty = true;
    }
}

void TimerMgr::start() {
    if (state != TMR_RUNNING && remainingTime_cs > 0) {
        // El Ticker se configura a 10ms (100 veces por segundo)
        timerTicker.attach_ms(10, onTick);
        state = TMR_RUNNING;
        dirty = true;
    }
}

void TimerMgr::pause() {
    if (state == TMR_RUNNING) {
        timerTicker.detach();
        state = TMR_PAUSED;
        dirty = true;
    }
}

void TimerMgr::reset() {
    timerTicker.detach();
    state = TMR_STOPPED;
    remainingTime_cs = initialTime_cs;
    dirty = true; // Forzar repintado
}

void TimerMgr::update() {
    if (state == TMR_EXPIRED) {
        // Si el tiempo ha expirado, hacemos sonar la alarma
        AlarmMgr::soundAlarm();
        // Cambiamos a STOPPED para que la alarma no se dispare continuamente
        state = TMR_STOPPED; 
    }

    if (dirty) {
        DisplayMgr::printMatrix(getFormattedTime());
        dirty = false;
    }
}

TimerState TimerMgr::getState() {
    return state;
}

TimerPrecision TimerMgr::getPrecision() {
    return currentPrecision;
}

String TimerMgr::getFormattedTime() {
    unsigned long time_cs = remainingTime_cs;
    char buffer[12];

    if (currentPrecision == PRECISION_HMS) {
        // Formato HH:MM:SS
        unsigned long total_s = time_cs / 100;
        int h = total_s / 3600;
        int m = (total_s % 3600) / 60;
        int s = total_s % 60;
        sprintf(buffer, "%02d:%02d:%02d", h, m, s);
    } else { // PRECISION_MS_CS
        // Formato MM:SS:CS
        unsigned long total_s = time_cs / 100;
        int m = total_s / 60;
        int s = total_s % 60;
        int cs = time_cs % 100;
        sprintf(buffer, "%02d:%02d:%02d", m, s, cs);
    }
    return String(buffer);
}

// --- Métodos Privados ---

void TimerMgr::onTick() {
    if (remainingTime_cs > 0) {
        remainingTime_cs--;
        dirty = true;

        // Optimización: solo redibujar si el segundo cambia en modo HMS
        if (currentPrecision == PRECISION_HMS && remainingTime_cs % 100 != 0) {
            dirty = false;
        }

    } else {
        // El tiempo ha llegado a cero
        timerTicker.detach();
        state = TMR_EXPIRED;
        dirty = true;
    }
}