#include "GlobalSettings.h"

// Definición e inicialización de las variables estáticas

// --- Reloj ---
bool GlobalSettings::use24hFormat;
int  GlobalSettings::timeZoneOffset;
ClockSource GlobalSettings::clockSource;

// --- Alarma ---
AlarmConfig GlobalSettings::alarms[3];
int  GlobalSettings::alarmVolume;
int  GlobalSettings::alarmToneIndex;

// --- Conectividad ---
bool GlobalSettings::wifiEnabled;

// --- Hardware y Brillo ---
BrightnessMode GlobalSettings::brightnessMode;
int  GlobalSettings::matrixBrightness;
