#include <Arduino.h>
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "CronoMgr.h" // <--- ¡Nuevo módulo!

void setup() {
    Serial.begin(115200);
    
    // 1. Inicializar Módulos
    DisplayMgr::init();
    InputMgr::init();
    CronoMgr::init();

    // Mensaje de bienvenida
    DisplayMgr::showMenuState("MODO PRUEBA", "CRONOMETRO");
    delay(1000);
    
    // Estado inicial visual
    CronoMgr::reset(); 
}

void loop() {
    // 1. Mantener Matrix viva
    DisplayMgr::updateRoutine();

    // 2. Lógica del Cronómetro (Dibuja el tiempo si está corriendo)
    CronoMgr::update();

    // 3. Leer Botones
    ButtonState btn = InputMgr::readButtons();

    if (btn != BTN_NONE) {
        // --- CONTROL SIMPLE DEL CRONÓMETRO ---
        
        // Botón OK: Alterna entre START y PAUSE
        if (btn == BTN_OK) {
            if (CronoMgr::getState() == RUNNING) {
                CronoMgr::pause();
            } else {
                CronoMgr::start();
            }
        }
        
        // Botón ABAJO: RESET (Solo si no está corriendo)
        if (btn == BTN_DOWN) {
             if (CronoMgr::getState() != RUNNING) {
                 CronoMgr::reset();
             }
        }
    }
}