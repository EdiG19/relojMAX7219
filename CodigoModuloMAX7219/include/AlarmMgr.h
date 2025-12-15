#ifndef ALARMMGR_H
#define ALARMMGR_H

#include <Arduino.h>
#include "GlobalSettings.h"
//#include "RTCMgr.h" // Necesitamos saber la hora actual para comparar

// Frecuencias de ejemplo para los tonos
#define TONE_FREQ_1 440 // La
#define TONE_FREQ_2 523 // Do
#define TONE_FREQ_3 659 // Mi

class AlarmMgr {
public:
    // Inicializa el pin del Buzzer (PWM)
    static void init();

    // Función principal a llamar en el loop() (Verifica si es hora de sonar)
    static void update(int currentHour, int currentMinute, int currentSecond);

    // --- Funciones para el Menú ---
    static void setTime(int alarmIndex, int hour, int minute);
    static void setTone(int alarmIndex, int toneIndex); // <-- Nuevo
    static void toggle(int alarmIndex, bool enabled);
    static void setVolume(int volume);

    // --- Control de Sonido ---
    static void soundAlarm(); // Para módulos externos (Timer) - usa un tono genérico
    static void previewTone(int toneIndex);
    static void stop(); 
    static bool isSounding();

private:
    static void playTone(int toneIndex);
    static void stopTone();

    // Variables internas
    static bool isCurrentlySounding;
    static unsigned long alarmStartTime;
};

#endif