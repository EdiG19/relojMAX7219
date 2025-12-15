#include "EepromMgr.h"
#include <EEPROM.h>

// --- ESTRUCTURA DE DATOS PARA GUARDAR ---
// Contiene una copia de todas las variables de GlobalSettings que deben persistir.
struct PersistentSettings {
    uint32_t magic_number;
    uint16_t version;

    // --- Reloj ---
    bool use24hFormat;
    int  timeZoneOffset;
    ClockSource clockSource;

    // --- Crono/Timer ---
    TimePrecision cronoPrecision;
    uint32_t timerInitialValue;

    // --- Alarmas ---
    AlarmConfig alarms[3];
    uint8_t alarmVolume;

    // --- Conectividad ---
    bool wifiEnabled;

    // --- Hardware ---
    BrightnessMode brightnessMode;
    uint8_t  matrixBrightness;
};

// --- VALORES DE CONTROL ---
const uint32_t MAGIC_NUMBER = 0xDEADBEEF; // Número para identificar nuestros datos
const uint16_t SETTINGS_VERSION = 2;      // ¡IMPORTANTE! Incrementado por cambio de estructura


void EepromMgr::loadSettings() {
    PersistentSettings settings_from_eeprom;
    EEPROM.get(0, settings_from_eeprom);

    // Verificamos si los datos son válidos
    if (settings_from_eeprom.magic_number == MAGIC_NUMBER && settings_from_eeprom.version == SETTINGS_VERSION) {
        // Datos válidos, los cargamos en GlobalSettings
        GlobalSettings::use24hFormat     = settings_from_eeprom.use24hFormat;
        GlobalSettings::timeZoneOffset   = settings_from_eeprom.timeZoneOffset;
        GlobalSettings::clockSource      = settings_from_eeprom.clockSource;
        GlobalSettings::cronoPrecision   = settings_from_eeprom.cronoPrecision;
        GlobalSettings::timerInitialValue = settings_from_eeprom.timerInitialValue;
        GlobalSettings::alarmVolume      = settings_from_eeprom.alarmVolume;
        GlobalSettings::wifiEnabled      = settings_from_eeprom.wifiEnabled;
        GlobalSettings::brightnessMode   = settings_from_eeprom.brightnessMode;
        GlobalSettings::matrixBrightness = settings_from_eeprom.matrixBrightness;

        for (int i = 0; i < 3; i++) {
            GlobalSettings::alarms[i] = settings_from_eeprom.alarms[i];
        }

    } else {
        // Datos no válidos (primer boot, EEPROM corrupta o versión antigua)
        // 1. Cargamos valores por defecto
        GlobalSettings::init();
        // 2. Guardamos esta configuración por defecto en EEPROM para el próximo reinicio
        saveSettings();
    }
}

void EepromMgr::saveSettings() {
    PersistentSettings settings_to_save;

    // Rellenamos la estructura con los valores actuales de GlobalSettings
    settings_to_save.magic_number    = MAGIC_NUMBER;
    settings_to_save.version         = SETTINGS_VERSION;
    settings_to_save.use24hFormat     = GlobalSettings::use24hFormat;
    settings_to_save.timeZoneOffset   = GlobalSettings::timeZoneOffset;
    settings_to_save.clockSource      = GlobalSettings::clockSource;
    settings_to_save.cronoPrecision   = GlobalSettings::cronoPrecision;
    settings_to_save.timerInitialValue = GlobalSettings::timerInitialValue;
    settings_to_save.alarmVolume      = GlobalSettings::alarmVolume;
    settings_to_save.wifiEnabled      = GlobalSettings::wifiEnabled;
    settings_to_save.brightnessMode   = GlobalSettings::brightnessMode;
    settings_to_save.matrixBrightness = GlobalSettings::matrixBrightness;

    for (int i = 0; i < 3; i++) {
        settings_to_save.alarms[i] = GlobalSettings::alarms[i];
    }

    // Escribimos la estructura completa en la EEPROM y confirmamos
    EEPROM.put(0, settings_to_save);
    EEPROM.commit();
}
