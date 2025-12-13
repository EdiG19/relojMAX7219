#include <Arduino.h>
#include "DisplayMgr.h"

void setup() {
    Serial.begin(115200);
    
    // Inicializamos con una sola línea. Limpio y elegante.
    DisplayMgr::init();

    delay(2000);
    
    // Prueba de Menú en LCD
    DisplayMgr::showMenuState("MENU PRINCIPAL", "> 1. Cronometro");
    
    // Prueba de Texto en Matriz
    DisplayMgr::printMatrix("TEST");
}

void loop() {
    // IMPORTANTE: Mantener la matriz viva
    DisplayMgr::updateRoutine();

    // Simulación: Cambiar texto cada 2 segundos para ver que funciona
    static unsigned long timer = 0;
    static int contador = 0;

    if (millis() - timer > 2000) {
        timer = millis();
        contador++;

        // Simular movimiento en menú LCD
        DisplayMgr::printLCD(1, "> Opcion " + String(contador));

        // Simular reloj en Matriz
        String horaFake = String(contador) + ":00";
        DisplayMgr::printMatrix(horaFake);
    }
}