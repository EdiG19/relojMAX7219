<<<<<<< HEAD
=======
#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <Arduino.h>
#define PIN_BUZZER 33 // Pin PWM 

// --- ENUMS DE CONFIGURACIÓN ---

enum ClockSource {
    SOURCE_WIFI_NTP,
    SOURCE_RTC,
    SOURCE_MANUAL
};

enum BrightnessMode {
    BRIGHTNESS_MANUAL,
    BRIGHTNESS_AUTO,
    BRIGHTNESS_NIGHT
};

// Precisión para Crono/Timer (afecta al refresco y display)
enum TimePrecision {
    PRECISION_SECONDS, // SS
    PRECISION_CENTIS   // SS:cs
};


// --- ESTADOS GLOBALES DE LA APLICACIÓN (MÁQUINA DE ESTADOS) ---
enum AppState {
    // Estados principales
    STATE_CLOCK,
    STATE_CRONO,
    STATE_TIMER,

    // Menús
    STATE_MENU_MAIN,
    STATE_MENU_WIFI,
    STATE_MENU_CLOCK,
    STATE_MENU_CRONO,
    STATE_MENU_TIMER,
    STATE_MENU_ALARM_LIST,
    STATE_MENU_BRIGHTNESS,

    // Estados de edición/procesos
    STATE_WIFI_CONNECTING,
    STATE_EDIT_MANUAL_TIME,
    STATE_EDIT_TIMER,
    STATE_EDIT_ALARM,
    STATE_EDIT_ALARM_TIME,
    STATE_EDIT_ALARM_TONE,
    STATE_EDIT_ALARM_VOLUME,
    STATE_EDIT_BRIGHTNESS
};

// --- ESTRUCTURAS DE DATOS ---

>>>>>>> 646555413e0a63ad482b81c74405e95396c979ba
struct AlarmConfig {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    uint8_t tone;
};

class GlobalSettings {
public:
    static AlarmConfig alarms[3];
    static uint8_t alarmVolume;
    static bool wifiEnabled;
    static BrightnessMode brightnessMode;
    static uint8_t matrixBrightness;
};
