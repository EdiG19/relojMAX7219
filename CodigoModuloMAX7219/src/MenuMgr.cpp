#include "MenuMgr.h"
#include "GlobalSettings.h" // Para acceder a los estados de la app
#include "CronoMgr.h"
#include "TimerMgr.h"

// --- Declaración de las funciones que se llamarán desde el menú ---
// Estas funciones simplemente cambian el estado global de la aplicación.
// La lógica de cada modo (qué hacer en STATE_CRONO, etc.) se manejará en main.cpp
void enterClockMode()   { GlobalSettings::appState = STATE_CLOCK; }
void enterCronoMode()   { CronoMgr::init(); GlobalSettings::appState = STATE_CRONO; }
void enterTimerMode()   { TimerMgr::init(); GlobalSettings::appState = STATE_TIMER; }
void enterAlarmsMenu()  { GlobalSettings::appState = STATE_MENU_ALARM; }
void enterConfigMenu()  { GlobalSettings::appState = STATE_MENU_CONFIG; }

// --- Definición de los ítems del Menú Principal ---
MenuItem MenuMgr::menuItems[] = {
    {"1. Ver Reloj",     enterClockMode},
    {"2. Cronometro",    enterCronoMode},
    {"3. Temporizador",  enterTimerMode},
    {"4. Alarmas",       enterAlarmsMenu},
    {"5. Configuracion", enterConfigMenu}
};

// --- Inicialización de variables estáticas ---
const int MenuMgr::itemsCount = sizeof(MenuMgr::menuItems) / sizeof(MenuItem);
int MenuMgr::currentIndex = 0;
int MenuMgr::topVisibleIndex = 0;


void MenuMgr::init() {
    currentIndex = 0;
    topVisibleIndex = 0;
}

void MenuMgr::handleNavigation(ButtonState btn) {
    if (btn == BTN_NONE) return;

    if (btn == BTN_DOWN) {
        currentIndex++;
        if (currentIndex >= itemsCount) {
            currentIndex = 0; // Vuelve al inicio
        }
    } else if (btn == BTN_UP) {
        currentIndex--;
        if (currentIndex < 0) {
            currentIndex = itemsCount - 1; // Vuelve al final
        }
    }

    // Lógica para el scroll del LCD
    if (currentIndex < topVisibleIndex) {
        topVisibleIndex = currentIndex; // Scroll hacia arriba
    } else if (currentIndex >= topVisibleIndex + LCD_ROWS) {
        topVisibleIndex = currentIndex - LCD_ROWS + 1; // Scroll hacia abajo
    }

    draw(); // Redibujar el menú después de cada cambio
}

void MenuMgr::executeAction() {
    // Llama a la función asociada al ítem actual (ej: enterCronoMode)
    if (menuItems[currentIndex].action != nullptr) {
        menuItems[currentIndex].action();
    }
}

void MenuMgr::draw() {
    DisplayMgr::clearLCD();

    // Dibuja los ítems visibles en el LCD (máximo LCD_ROWS)
    for (int i = 0; i < LCD_ROWS; i++) {
        int itemIndex = topVisibleIndex + i;

        if (itemIndex < itemsCount) {
            String line = "";
            
            // Añade un cursor ">" al ítem seleccionado
            if (itemIndex == currentIndex) {
                line += ">";
            } else {
                line += " ";
            }

            line += menuItems[itemIndex].title;
            DisplayMgr::printLCD(i, line);
        }
    }
}