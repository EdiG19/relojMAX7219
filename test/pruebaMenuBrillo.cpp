#include <Arduino.h>
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "GlobalSettings.h" // Necesario para los modos de brillo

// --- ESTADOS DE LA APLICACIÓN ---
enum AppState {
    STATE_MENU,
    STATE_MANUAL_ADJUST
};

AppState currentState = STATE_MENU;
int menuSelection = 0; // 0: Auto, 1: Manual, 2: Noche

// --- FUNCIÓN PARA ACTUALIZAR LA PANTALLA DEL MENÚ ---
void updateMenuDisplay() {
    DisplayMgr::printLCD(0, "Modo Brillo:");
    String option = "";
    switch (menuSelection) {
        case 0: option = "> Auto"; break;
        case 1: option = "> Manual"; break;
        case 2: option = "> Noche"; break;
    }
    DisplayMgr::printLCD(1, option);
}

void setup() {
    Serial.begin(115200);
    
    // 1. Inicializar Módulos y Settings
    GlobalSettings::init();
    DisplayMgr::init();
    InputMgr::init();

    // 2. Estado inicial visual
    updateMenuDisplay();
    DisplayMgr::printMatrix("Hola!");
}

void loop() {
    // 1. Mantener Matrix viva
    DisplayMgr::updateRoutine();

    // 2. Leer Botones
    ButtonState btn = InputMgr::readButtons();

    // 3. Lógica de estados
    if (currentState == STATE_MENU) {
        // --- NAVEGACIÓN EN EL MENÚ ---
        if (btn == BTN_DOWN) {
            menuSelection = (menuSelection + 1) % 3;
            updateMenuDisplay();
        }
        if (btn == BTN_UP) {
            menuSelection = (menuSelection - 1 + 3) % 3;
            updateMenuDisplay();
        }
        if (btn == BTN_OK) {
            // Asignar el modo de brillo seleccionado
            if (menuSelection == 0) GlobalSettings::brightnessMode = BRIGHTNESS_AUTO;
            if (menuSelection == 1) GlobalSettings::brightnessMode = BRIGHTNESS_MANUAL;
            if (menuSelection == 2) GlobalSettings::brightnessMode = BRIGHTNESS_NIGHT;
            
            // Si entramos en modo manual, cambiamos de estado
            if (GlobalSettings::brightnessMode == BRIGHTNESS_MANUAL) {
                currentState = STATE_MANUAL_ADJUST;
                DisplayMgr::printLCD(0, "Ajuste Manual:");
            }
        }

    } else if (currentState == STATE_MANUAL_ADJUST) {
        // --- AJUSTE DE BRILLO MANUAL ---
        if (btn == BTN_UP) {
            GlobalSettings::matrixBrightness = min(15, GlobalSettings::matrixBrightness + 1);
        }
        if (btn == BTN_DOWN) {
            GlobalSettings::matrixBrightness = max(0, GlobalSettings::matrixBrightness - 1);
        }
        if (btn == BTN_BACK) {
            currentState = STATE_MENU; // Volver al menú
            updateMenuDisplay();
        }
        // Mostrar el nivel de brillo actual
        DisplayMgr::printLCD(1, "Nivel: " + String(GlobalSettings::matrixBrightness));
    }


    // 4. Aplicar configuración de brillo (se ejecuta siempre, en cada loop)
    switch (GlobalSettings::brightnessMode) {
        
        case BRIGHTNESS_AUTO:
        {
            int autoBrightness = InputMgr::getAutoBrightness();
            DisplayMgr::setMatrixBrightness(autoBrightness);
            Serial.println(autoBrightness);
            // Si hay muy poca luz (valor < 2), apagamos el LCD. Si no, lo encendemos.
            //DisplayMgr::setLcdBacklight(autoBrightness >= 2);
            
            if (currentState != STATE_MENU) { // Si venimos de otro modo, volvemos al menú
                currentState = STATE_MENU;
                updateMenuDisplay();
            }
            break;
        }
            
        case BRIGHTNESS_MANUAL:
            DisplayMgr::setMatrixBrightness(GlobalSettings::matrixBrightness);
            DisplayMgr::setLcdBacklight(true); // En modo manual, LCD siempre encendido
            break;

        case BRIGHTNESS_NIGHT:
            DisplayMgr::setMatrixBrightness(0); // Mínimo brillo
            DisplayMgr::setLcdBacklight(false); // LCD apagado
            
            if (currentState != STATE_MENU) { // Si venimos de otro modo, volvemos al menú
                currentState = STATE_MENU;
                updateMenuDisplay();
            }
            break;
    }
    
    delay(10); // Pequeña pausa para estabilizar
}
