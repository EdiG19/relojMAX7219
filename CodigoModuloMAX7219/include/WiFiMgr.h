#ifndef WIFIMGR_H
#define WIFIMGR_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "time.h"

// Configuración de Zona Horaria (Panamá/Colombia/Perú = UTC -5)
#define GMT_OFFSET_SEC  -18000 
#define DAYLIGHT_OFFSET 0      
#define NTP_SERVER      "pool.ntp.org"

class WiFiMgr {
public:
    // Intenta conectar usando credenciales guardadas. No bloquea (mucho).
    static void init();

    // Lanza el Portal de Configuración (bloqueante)
    // Útil cuando seleccionas "Configurar WiFi" en el menú.
    static void startConfigPortal();

    // Verifica si estamos conectados
    static bool isConnected();

    // Devuelve la hora formateada "HH:MM" (o "HH:MM:SS" si quieres)
    // Si no ha sincronizado, devuelve una cadena vacía o error.
    static String getLocalTimeStr();

private:
    static void setupTime(); // Configura NTP interno
};

#endif