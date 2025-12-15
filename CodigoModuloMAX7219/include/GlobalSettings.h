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
    STATE_MENU_MAIN,

    // Sub-menús de configuración
    STATE_MENU_WIFI,
    STATE_MENU_CLOCK,
    STATE_MENU_ALARM_LIST,
    STATE_MENU_ALARM_EDIT,
    STATE_MENU_BRIGHTNESS,
    STATE_MENU_TIMER,
    STATE_MENU_CRONO,

    // Estados de edición (pantallas específicas para cambiar un valor)
    STATE_EDIT_MANUAL_TIME,
    STATE_EDIT_TIMER,
    STATE_EDIT_ALARM,
    STATE_EDIT_BRIGHTNESS,
    STATE_WIFI_CONNECTING
};

// --- ESTRUCTURAS DE DATOS ---

struct AlarmConfig {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    uint8_t tone; // Tono para esta alarma específica
};


class GlobalSettings {
public:
    // --- Configuración de Reloj ---
    static bool use24hFormat;
    static int  timeZoneOffset;
    static ClockSource clockSource;

    // --- Configuración de Crono/Timer ---
    static TimePrecision cronoPrecision;
    static uint32_t timerInitialValue; // en segundos

    // --- Configuración de Alarma ---
    static AlarmConfig alarms[3];
    static uint8_t alarmVolume; // 0 a 10

    // --- Configuración de Conectividad ---
    static bool wifiEnabled;

    // --- Configuración de Hardware ---
    static BrightnessMode brightnessMode;
    static uint8_t  matrixBrightness;

    // --- Estado de la Aplicación (no persistente) ---
    static AppState appState;
    static int editingIndex; // Índice para saber qué alarma/item estamos editando

    // Inicializa valores por defecto
    static void init() {
        use24hFormat = true;
        timeZoneOffset = -18000;
        clockSource = SOURCE_RTC;
        wifiEnabled = true;

        cronoPrecision = PRECISION_CENTIS;
        timerInitialValue = 60; // 1 minuto por defecto

        alarmVolume = 5;
        for (int i = 0; i < 3; i++) {
            alarms[i] = {7, 0, false, 0};
        }

        brightnessMode = BRIGHTNESS_AUTO;
        matrixBrightness = 5; // Brillo intermedio por defecto (0-15)

        appState = STATE_CLOCK;
        editingIndex = 0;
    }
};

#endif