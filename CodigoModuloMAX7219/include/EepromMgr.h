#ifndef EEPROMMGR_H
#define EEPROMMGR_H

#include "GlobalSettings.h"

// Tamaño total en bytes que reservaremos en la EEPROM.
// Debe ser suficiente para guardar toda la estructura de GlobalSettings.
// sizeof(bool) + sizeof(int) + sizeof(ClockSource) + 3*sizeof(AlarmConfig) + etc...
// Seamos generosos para futuras ampliaciones.
#define EEPROM_SIZE 128 

class EepromMgr {
public:
    // Carga la configuración desde la EEPROM a las variables de GlobalSettings.
    // Si la EEPROM está vacía o corrupta, carga los valores por defecto y los guarda.
    static void loadSettings();

    // Guarda la configuración actual de GlobalSettings en la EEPROM.
    static void saveSettings();
};

#endif // EEPROMMGR_H
