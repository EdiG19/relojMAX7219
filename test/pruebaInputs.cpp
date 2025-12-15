#include <Arduino.h>
#include "DisplayMgr.h"
#include "InputMgr.h" // <--- ¡Nuevo!

void setup() {
    Serial.begin(115200);
    
    // Iniciamos Pantallas y Entradas
    DisplayMgr::init();
    InputMgr::init();

    DisplayMgr::showMenuState("TEST BOTONES", "Presiona uno...");
}

void loop() {
    // Mantener la matriz viva
    DisplayMgr::updateRoutine();

    // 1. LEER BOTONES
    ButtonState btn = InputMgr::readButtons();

    // 2. ACTUAR SEGÚN EL BOTÓN
    if (btn != BTN_NONE) {
        // Solo entramos aquí si se presionó algo
        Serial.println("Boton detectado!");

        switch (btn) {
            case BTN_UP:
                DisplayMgr::printLCD(1, "Boton: ARRIBA");
                DisplayMgr::printMatrix("UP");
                break;
            case BTN_DOWN:
                DisplayMgr::printLCD(1, "Boton: ABAJO");
                DisplayMgr::printMatrix("DOWN");
                break;
            case BTN_OK:
                DisplayMgr::printLCD(1, "Boton: OK/ENT");
                DisplayMgr::printMatrix("OK");
                break;
            case BTN_BACK:
                DisplayMgr::printLCD(1, "Boton: ATRAS");
                DisplayMgr::printMatrix("BACK");
                break;
            default:
                break;
        }
    }

    // 3. PRUEBA LDR (Opcional, descomentar para ver valores en Serial)
    
    static unsigned long ldrTimer = 0;
    if (millis() - ldrTimer > 1000) {
        ldrTimer = millis();
        int luz = InputMgr::getAutoBrightness();
        Serial.print("Nivel Luz: "); Serial.println(luz);
        DisplayMgr::setMatrixBrightness(luz); // Prueba visual de auto-brillo
    }
}
