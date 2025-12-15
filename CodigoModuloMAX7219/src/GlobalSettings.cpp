#include "GlobalSettings.h"

// --- Alarmas ---
AlarmConfig GlobalSettings::alarms[3];
uint8_t GlobalSettings::alarmVolume;

// --- Conectividad ---
bool GlobalSettings::wifiEnabled;

// --- Hardware ---
BrightnessMode GlobalSettings::brightnessMode;
uint8_t GlobalSettings::matrixBrightness;
