#include "GlobalSettings.h"

// ==========================================
// DEFINICIÓN DE VARIABLES ESTÁTICAS
// ==========================================

// --- Reloj ---
// En .h: static bool use24hFormat;
bool GlobalSettings::use24hFormat;

// En .h: static int timeZoneOffset;
int  GlobalSettings::timeZoneOffset;

// En .h: static ClockSource clockSource;
ClockSource GlobalSettings::clockSource;


// --- Crono/Timer ---
// En .h: static TimePrecision cronoPrecision;
TimePrecision GlobalSettings::cronoPrecision;

// En .h: static uint32_t timerInitialValue;
uint32_t GlobalSettings::timerInitialValue;


// --- Alarma ---
// En .h: static AlarmConfig alarms[3];
AlarmConfig GlobalSettings::alarms[3];

// En .h: static uint8_t alarmVolume; 
// ¡CORRECCIÓN AQUÍ! Debe ser uint8_t, no int
uint8_t GlobalSettings::alarmVolume; 

// NOTA: alarmToneIndex SE ELIMINÓ porque ahora el tono
// es parte de la estructura AlarmConfig (alarms[i].tone).


// --- Conectividad ---
// En .h: static bool wifiEnabled;
bool GlobalSettings::wifiEnabled;


// --- Hardware y Brillo ---
// En .h: static BrightnessMode brightnessMode;
BrightnessMode GlobalSettings::brightnessMode;

// En .h: static uint8_t matrixBrightness;
// ¡CORRECCIÓN AQUÍ! Debe ser uint8_t, no int
uint8_t GlobalSettings::matrixBrightness;


// --- Estado de la App ---
// En .h: static AppState appState;
AppState GlobalSettings::appState;

// En .h: static int editingIndex;
int GlobalSettings::editingIndex;