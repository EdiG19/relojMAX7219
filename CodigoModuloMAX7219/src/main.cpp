#include <Arduino.h>
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "RTCMgr.h" 

// Nota: Ya no incluimos WiFiMgr

void setup() {
    Serial.begin(115200);
    
    // 1. Inicializar I2C y Pantallas
    DisplayMgr::init(); 
    //DisplayMgr::showMenuState("INICIANDO...", "Hardware Check");
    delay(1000);

    // 2. Inicializar RTC con Makuna
    if (!RTCMgr::init()) {
        Serial.println("Fallo RTC");
        //DisplayMgr::showMenuState("ERROR HARDWARE", "Fallo RTC I2C");
        while(1); // Bloqueo infinito si el hardware falla
    }
    
    // Mensaje de éxito
    DisplayMgr::printMatrix("RTC OK");
    //DisplayMgr::printLCD(0, "RTC
    delay(1000);
}

void loop() {
    // 1. VITAL: Mantener la matriz viva (Parola animation)
    // Si quitas esto o pones un delay grande, la matriz falla.
    DisplayMgr::updateRoutine();

    // 2. Lógica de actualización cada 1 segundo (usando millis)
    static unsigned long lastUpdate = 0;
    
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis(); // Reiniciamos el cronómetro

        // A. Obtener datos del RTC
        RtcDateTime now = RTCMgr::now();
        float temp = RTCMgr::getTemperature();

        // B. Formatear la hora para la Matriz (HH:MM:SS)
        char timeBuffer[10];
        sprintf(timeBuffer, "%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());
        
        // ¡Aquí mostramos en el MAX7219!
        DisplayMgr::printMatrix(String(timeBuffer));

        // C. Formatear Fecha y Temp para el LCD (Información secundaria)
        char dateBuffer[17];
        sprintf(dateBuffer, "%02d/%02d %.1fC", now.Day(), now.Month(), temp);

        DisplayMgr::printLCD(0, "RELOJ RTC"); // Título fijo
        DisplayMgr::printLCD(1, String(dateBuffer)); // Fecha y Temp
    }
}