#include <Arduino.h>
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
//#include "WiFiMgr.h"
#include "RTCMgr.h" // <--- Incluimos el nuevo Manager

void setup() {
    Serial.begin(115200);
    
    // 1. Inicializar I2C y Pantallas
    DisplayMgr::init(); 
    DisplayMgr::showMenuState("INICIANDO...", "Hardware Check");
    delay(1000);

    // 2. Inicializar RTC con Makuna
    if (!RTCMgr::init()) {
        Serial.println("Fallo RTC");
        DisplayMgr::showMenuState("ERROR HARDWARE", "Fallo RTC I2C");
        while(1); // Bloqueo si falla
    }
    
    // Mensaje de éxito
    DisplayMgr::printLCD(1, "RTC: OK");
    delay(1000);

    // 3. Inicializar WiFi (Opcional, para sincronizar)
  //  WiFiMgr::init();
    
    // Sincronización NTP -> RTC (Si hay WiFi)
    /*if (WiFiMgr::isConnected()) {
       DisplayMgr::printLCD(1, "Sync NTP...");
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 2000)) {
            // Convertimos 'tm' a 'RtcDateTime' de Makuna
            RtcDateTime nowNTP(
                timeinfo.tm_year + 1900, 
                timeinfo.tm_mon + 1, 
                timeinfo.tm_mday, 
                timeinfo.tm_hour, 
                timeinfo.tm_min, 
                timeinfo.tm_sec
            );
            RTCMgr::adjust(nowNTP);
            DisplayMgr::printLCD(1, "RTC Actualizado!");
        } else {
            DisplayMgr::printLCD(1, "Fallo NTP");
        }
        delay(1000); */
    //}
}

void loop() {
    // Limpiamos pantalla de LCD para refresco
    // (Nota: En un loop real no debes limpiar todo el tiempo porque parpadea,
    //  pero para esta prueba de visualización simple está bien).
    
    // 1. Obtener la hora del RTC (Makuna)
    RtcDateTime now = RTCMgr::now();
    float temp = RTCMgr::getTemperature();

    // 2. Formatear la hora
    // Makuna usa métodos con Mayúscula: Hour(), Minute(), Second()
    char timeBuffer[16];
    sprintf(timeBuffer, "%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());

    // 3. Formatear la fecha y temperatura
    char dateBuffer[16];
    sprintf(dateBuffer, "%02d/%02d %.1fC", now.Day(), now.Month(), temp);

    // 4. Mostrar en LCD
    DisplayMgr::printLCD(0, String(timeBuffer)); // Fila 0: Hora
    DisplayMgr::printLCD(1, String(dateBuffer)); // Fila 1: Fecha y Temp

    // Pausa de 1 segundo para no saturar
    delay(1000);
}