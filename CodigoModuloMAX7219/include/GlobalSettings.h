#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>

// Definimos las posibles fuentes de tiempo
enum ClockSource {
    SOURCE_WIFI_NTP, // Prioridad: Internet
    SOURCE_RTC,      // Prioridad: Módulo físico (DS3231)
    SOURCE_MANUAL    // Sin módulo, el tiempo que cuenta el ESP32 internamente
};

class GlobalSettings {
public:
    // --- Configuración de Reloj ---
    static bool use24hFormat;     
    static int  timeZoneOffset;   
    static ClockSource clockSource; // <--- NUEVO: ¿Quién manda en la hora?

    // --- Configuración de Alarma ---
    static int  alarmVolume;      // 0 a 10 (para PWM del buzzer)
    static int  alarmToneIndex;   // 0=Beep, 1=Melodía 1, etc.
    static bool alarmEnabled;     // Alarma activada o no
    static int  alarmHour;
    static int  alarmMinute;

    // --- Configuración de Hardware ---
    static int  matrixBrightness; 
    static bool autoBrightness;   

    // Inicializa valores por defecto
    static void init() {
        use24hFormat = true;
        timeZoneOffset = -18000;
        
        // Por defecto intentamos WiFi, si falla, el sistema cambiará a RTC automáticamente
        clockSource = SOURCE_WIFI_NTP; 

        // Valores alarma por defecto
        alarmVolume = 5;       // Volumen medio
        alarmToneIndex = 0;    // Tono simple
        alarmEnabled = false;
        alarmHour = 7;
        alarmMinute = 0;

        matrixBrightness = 1;
        autoBrightness = true;
    }
};

#endif