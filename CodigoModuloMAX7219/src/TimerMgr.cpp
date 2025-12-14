#include "TimerMgr.h"
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "AlarmMgr.h"

// Inicialización del puntero a la instancia estática
TimerMgr* TimerMgr::instance = nullptr;

TimerMgr::TimerMgr() {
    // Asignar la instancia actual al puntero estático
    instance = this;
}

void TimerMgr::init() {
    this->currentPrecision = PRECISION_HMS; // Precisión por defecto
    this->state = TMR_STOPPED;
    this->initialTime_cs = 6000; // Por defecto 1 minuto (60s * 100)
    this->remainingTime_cs = this->initialTime_cs;
    this->dirty = true;
}

void TimerMgr::setPrecision(TimerPrecision precision) {
    this->currentPrecision = precision;
    this->dirty = true; // Forzar redibujo para mostrar el nuevo formato
}

void TimerMgr::setInitialTime(unsigned long seconds) {
    // Detenemos el temporizador para cambiar el tiempo de forma segura
    if (this->state == TMR_RUNNING) {
        pause();
    }
    this->initialTime_cs = seconds * 100;
    this->remainingTime_cs = this->initialTime_cs;
    this->dirty = true;
}

void TimerMgr::start() {
    if (state == TMR_STOPPED || state == TMR_PAUSED) {
        state = TMR_RUNNING;
        // La interrupción se ejecuta cada 10ms (1cs)
        timerTicker.attach_ms(10, tick_wrapper);
    }
}

void TimerMgr::pause() {
    if (state == TMR_RUNNING) {
        state = TMR_PAUSED;
        timerTicker.detach();
    }
}

void TimerMgr::reset() {
    timerTicker.detach();
    state = TMR_STOPPED;
    remainingTime_cs = initialTime_cs;
    AlarmMgr::stopTone(); // Si estaba sonando, lo para
    dirty = true;
}

TimerState TimerMgr::getState() {
    return state;
}

// Wrapper estático que llama al método de instancia
void TimerMgr::tick_wrapper() {
    if (instance) {
        instance->tick();
    }
}

void TimerMgr::tick() {
    if (remainingTime_cs > 0) {
        remainingTime_cs--;
        // No marcamos 'dirty' aquí para no sobrecargar el loop principal desde una ISR.
        // El loop principal se encargará de redibujar periódicamente.
    } else {
        state = TMR_EXPIRED;
        timerTicker.detach(); // Paramos el ticker
        AlarmMgr::playTone(GlobalSettings::alarmToneIndex); // Hacemos sonar la alarma
        dirty = true; // Forzamos un último redibujo a "00:00:00"
    }
}

String TimerMgr::getFormattedTime() {
    unsigned long total_cs = remainingTime_cs;
    char buffer[12];

    if (currentPrecision == PRECISION_HMS) {
        unsigned long total_s = total_cs / 100;
        int h = total_s / 3600;
        int m = (total_s % 3600) / 60;
        int s = total_s % 60;
        sprintf(buffer, "%02d:%02d:%02d", h, m, s);
    } else { // PRECISION_MS_CS
        int cs = total_cs % 100;
        unsigned long total_s = total_cs / 100;
        int m = total_s / 60;
        int s = total_s % 60;
        sprintf(buffer, "%02d:%02d:%02d", m, s, cs);
    }
    return String(buffer);
}

TimerPrecision TimerMgr::getPrecision() {
    return currentPrecision;
}

void TimerMgr::update() {
    // Forzamos un redibujo periódico cuando está corriendo para ver los centisegundos
    if (state == TMR_RUNNING && currentPrecision == PRECISION_MS_CS) {
        dirty = true;
    }
    
    if (dirty) {
        String timeStr = getFormattedTime();
        DisplayMgr::printMatrix(timeStr.c_str());
        
        if (state == TMR_EXPIRED) {
            DisplayMgr::printLCD(0, "¡TIEMPO!");
            DisplayMgr::printLCD(1, "Pulsa OK");
        }
        
        dirty = false;
    }
}