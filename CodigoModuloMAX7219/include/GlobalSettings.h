#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>
#define PIN_BUZZER 33 // Pin PWM 

// Definimos las posibles fuentes de tiempo
enum ClockSource {
    SOURCE_WIFI_NTP, // Prioridad: Internet
    SOURCE_RTC,      // Prioridad: Módulo físico (DS3231)
    SOURCE_MANUAL    // Sin módulo, el tiempo que cuenta el ESP32 internamente
};
enum BrightnessMode{
    BRIGHTNESS_MANUAL,
    BRIGHTNESS_AUTO,
    BRIGHTNESS_NIGHT
};
struct AlarmConfig{
    int hour;
    int minute;
    bool enabled;
};
class GlobalSettings {
public:
    // --- Configuración de Reloj ---
    static bool use24hFormat;     
    static int  timeZoneOffset;   
    static ClockSource clockSource; // <--- NUEVO: ¿Quién manda en la hora?

    // --- Configuración de Alarma ---
    static AlarmConfig alarms[3];
    static int  alarmVolume;      // 0 a 10 (para PWM del buzzer)
    static int  alarmToneIndex;   // 0=Beep, 1=Melodía 1, etc.
    static bool wifiEnabled;

    // --- Configuración de Hardware ---
    static BrightnessMode brightnessMode;
    static int  matrixBrightness; 

    // Inicializa valores por defecto
    static void init() {
        use24hFormat = true;
        timeZoneOffset = -18000;
        wifiEnabled = true;
        
        // Por defecto intentamos WiFi, si falla, el sistema cambiará a RTC automáticamente
        clockSource = SOURCE_MANUAL; 

        // Valores alarma por defecto
        alarmVolume = 5;       // Volumen medio
        alarmToneIndex = 0;    // Tono simple
        
        for (int i = 0; i < 3; i++) {
            alarms[i] = {7, 0, false};
        }

        brightnessMode = BRIGHTNESS_AUTO;
        matrixBrightness = 1;
    }
};

#endif