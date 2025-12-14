#include <Arduino.h>
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "AlarmMgr.h"

// --- Estados del Menú ---
enum MenuState {
    MENU_MAIN,
    MENU_SET_VOLUME,
    MENU_SET_RINGTONE
};
MenuState currentMenuState = MENU_MAIN;
int mainMenuSelection = 0; // 0: Tiempo, 1: Volumen, 2: Ringtone
const int MAIN_MENU_OPTIONS = 3;
String menuItems[] = {"Tiempo", "Volumen", "Ringtone"};

// --- Simulación de Tiempo (simplificada para la prueba) ---
unsigned long lastTick = 0;
int currentHour = 0, currentMinute = 0, currentSecond = 0;

// --- Temporizador para la alarma de 10s ---
unsigned long countdownMillis = 0;
bool isCountingDown = false;

// --- Prototipos de funciones ---
void updateDisplay();
void handleInputs();

void setup() {
    Serial.begin(115200);
    GlobalSettings::init();
    DisplayMgr::init();
    InputMgr::init();
    AlarmMgr::init();

    DisplayMgr::setLcdBacklight(true);
    updateDisplay();
}

void loop() {
    // 1. Actualizar tiempo simulado
    if (millis() - lastTick >= 1000) {
        lastTick = millis();
        currentSecond++;
        if (currentSecond >= 60) {
            currentSecond = 0;
            currentMinute++;
            if (currentMinute >= 60) {
                currentMinute = 0;
                currentHour = (currentHour + 1) % 24;
            }
        }
    }

    // 2. Manejar el countdown para la matriz
    if (isCountingDown) {
        long remaining = (countdownMillis - millis()) / 1000;
        if (remaining >= 0) {
            DisplayMgr::printMatrix(String(remaining));
        } else {
            isCountingDown = false;
            DisplayMgr::printMatrix(""); // Limpiar matriz
        }
    }

    // 3. Manejar lógica de Alarma y Display
    AlarmMgr::update(currentHour, currentMinute, currentSecond);
    DisplayMgr::updateRoutine();

    // 4. Leer y procesar entradas de usuario
    handleInputs();
}

void updateDisplay() {
    String line1 = "", line2 = "";

    switch (currentMenuState) {
        case MENU_MAIN:
            line1 = "Alarma";
            // Poner un ">" en la opción seleccionada
            for (int i = 0; i < MAIN_MENU_OPTIONS; i++) {
                if (i == mainMenuSelection) {
                    line2 += ">";
                } else {
                    line2 += " ";
                }
                line2 += menuItems[i] + " ";
            }
            break;
            
        case MENU_SET_VOLUME:
            line1 = "Volumen";
            line2 = "< " + String(GlobalSettings::alarmVolume) + " >";
            break;

        case MENU_SET_RINGTONE:
            line1 = "Ringtone";
            line2 = "< Tono " + String(GlobalSettings::alarmToneIndex + 1) + " >";
            break;
    }
    DisplayMgr::showMenuState(line1, line2);
}

void handleInputs() {
    ButtonState button = InputMgr::readButtons();
    if (button == BTN_NONE) {
        return;
    }

    // Si la alarma está sonando, cualquier botón la detiene.
    if (AlarmMgr::isSounding()) {
        AlarmMgr::stop();
        DisplayMgr::printMatrix(""); // Limpiar cualquier mensaje de alarma
        updateDisplay(); // Restaurar el menú
        return;
    }

    switch (currentMenuState) {
        case MENU_MAIN:
            if (button == BTN_DOWN) {
                mainMenuSelection = (mainMenuSelection + 1) % MAIN_MENU_OPTIONS;
            } else if (button == BTN_UP) {
                mainMenuSelection = (mainMenuSelection - 1 + MAIN_MENU_OPTIONS) % MAIN_MENU_OPTIONS;
            } else if (button == BTN_OK) {
                // Acción al seleccionar una opción del menú principal
                if (mainMenuSelection == 0) { // Tiempo
                    // Configurar alarma para que suene en 10 segundos
                    int alarm_h = currentHour;
                    int alarm_m = currentMinute;
                    int alarm_s = currentSecond + 11; // +11 para dar margen
                    
                    if(alarm_s >= 60) {
                        alarm_s -= 60;
                        alarm_m++;
                        if(alarm_m >= 60) {
                            alarm_m = 0;
                            alarm_h = (alarm_h + 1) % 24;
                        }
                    }
                    AlarmMgr::setTime(0, alarm_h, alarm_m);
                    AlarmMgr::toggle(0, true); // Habilitar alarma 0

                    countdownMillis = millis() + 10000;
                    isCountingDown = true;
                    DisplayMgr::showMenuState("Alarma", "Activada en 10s");
                    delay(1500); // Pequeña pausa para que el usuario lea
                    //AlarmMgr::playTone(0);
                } else if (mainMenuSelection == 1) { // Volumen
                    currentMenuState = MENU_SET_VOLUME;
                } else if (mainMenuSelection == 2) { // Ringtone
                    currentMenuState = MENU_SET_RINGTONE;
                }
            }
            break;

        case MENU_SET_VOLUME:
            if (button == BTN_DOWN) {
                int vol = GlobalSettings::alarmVolume - 1;
                AlarmMgr::setVolume(vol);
            } else if (button == BTN_UP) {
                int vol = GlobalSettings::alarmVolume + 1;
                AlarmMgr::setVolume(vol);
            } else if (button == BTN_OK) {
                // Al presionar OK, hacer un preview del sonido
                AlarmMgr::previewTone(GlobalSettings::alarmToneIndex);
            } else if (button == BTN_BACK) {
                currentMenuState = MENU_MAIN;
            }
            break;

        case MENU_SET_RINGTONE:
            if (button == BTN_DOWN) {
                int tone = (GlobalSettings::alarmToneIndex + 1) % 3;
                AlarmMgr::selectRingtone(tone);
                AlarmMgr::previewTone(tone);
            } else if (button == BTN_UP) {
                int tone = (GlobalSettings::alarmToneIndex - 1 + 3) % 3;
                AlarmMgr::selectRingtone(tone);
                AlarmMgr::previewTone(tone);
            } else if (button == BTN_BACK) {
                currentMenuState = MENU_MAIN;
            }
            break;
    }
    
    // Actualizar la pantalla después de cada acción
    updateDisplay();
}