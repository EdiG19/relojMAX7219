#include "EepromMgr.h"
#include <EEPROM.h>

// --- ESTRUCTURA DE DATOS PARA GUARDAR ---
// Es una buena práctica crear una struct que contenga TODO lo que quieres guardar.
// Facilita la lectura/escritura en una sola operación con EEPROM.put() y EEPROM.get().
struct PersistentSettings {
    uint32_t magic_number; // Para verificar si la EEPROM ha sido inicializada por nuestro programa
    uint16_t version;      // Para manejar futuras actualizaciones de la estructura

    // --- Copia de las variables de GlobalSettings que queremos guardar ---
    bool use24hFormat;
    int  timeZoneOffset;
    ClockSource clockSource;
    AlarmConfig alarms[3];
    int  alarmVolume;
    int  alarmToneIndex;
    bool wifiEnabled;
    BrightnessMode brightnessMode;
    int  matrixBrightness;
};

// --- VALORES DE CONTROL ---
const uint32_t MAGIC_NUMBER = 0xDEADBEEF; // Un número aleatorio para identificar nuestra data
const uint16_t SETTINGS_VERSION = 1;      // Versión inicial de la estructura


void EepromMgr::loadSettings() {
    PersistentSettings settings_from_eeprom;

    // Leemos la estructura completa desde la EEPROM
    EEPROM.get(0, settings_from_eeprom);

    // Verificamos si los datos son válidos
    if (settings_from_eeprom.magic_number == MAGIC_NUMBER && settings_from_eeprom.version == SETTINGS_VERSION) {
        // Los datos son válidos, los cargamos en GlobalSettings
        GlobalSettings::use24hFormat     = settings_from_eeprom.use24hFormat;
        GlobalSettings::timeZoneOffset   = settings_from_eeprom.timeZoneOffset;
        GlobalSettings::clockSource      = settings_from_eeprom.clockSource;
        GlobalSettings::alarmVolume      = settings_from_eeprom.alarmVolume;
        GlobalSettings::alarmToneIndex   = settings_from_eeprom.alarmToneIndex;
        GlobalSettings::wifiEnabled      = settings_from_eeprom.wifiEnabled;
        GlobalSettings::brightnessMode   = settings_from_eeprom.brightnessMode;
        GlobalSettings::matrixBrightness = settings_from_eeprom.matrixBrightness;

        for (int i = 0; i < 3; i++) {
            GlobalSettings::alarms[i] = settings_from_eeprom.alarms[i];
        }

    } else {
        // Datos no válidos (primera vez, EEPROM corrupta o versión antigua)
        // 1. Cargamos los valores por defecto desde GlobalSettings::init()
        GlobalSettings::init();
        // 2. Guardamos esta configuración por defecto en la EEPROM para el próximo reinicio
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
    settings_to_save.alarmVolume      = GlobalSettings::alarmVolume;
    settings_to_save.alarmToneIndex   = GlobalSettings::alarmToneIndex;
    settings_to_save.wifiEnabled      = GlobalSettings::wifiEnabled;
    settings_to_save.brightnessMode   = GlobalSettings::brightnessMode;
    settings_to_save.matrixBrightness = GlobalSettings::matrixBrightness;

    for (int i = 0; i < 3; i++) {
        settings_to_save.alarms[i] = GlobalSettings::alarms[i];
    }

    // Escribimos la estructura completa en la EEPROM
    EEPROM.put(0, settings_to_save);
    EEPROM.commit(); // Aseguramos que los datos se escriban físicamente
}
