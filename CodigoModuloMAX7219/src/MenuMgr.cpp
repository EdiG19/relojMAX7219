#include "MenuMgr.h"
#include "GlobalSettings.h"
#include "CronoMgr.h"
#include "TimerMgr.h"

// --- Declaración de las funciones que se llamarán desde el menú ---
// Simplemente cambian el estado global. La lógica la lleva main.cpp
void enterWifiConfig()    { GlobalSettings::appState = STATE_MENU_WIFI; }
void enterClockConfig()   { GlobalSettings::appState = STATE_MENU_CLOCK; }
void enterCronoMode()     { GlobalSettings::appState = STATE_MENU_CRONO; }
void enterTimerMode()     { GlobalSettings::appState = STATE_MENU_TIMER; }
void enterAlarmsMenu()    { GlobalSettings::appState = STATE_MENU_ALARM_LIST; }
void enterBrightnessMenu(){ GlobalSettings::appState = STATE_MENU_BRIGHTNESS; }


// --- Definición de los ítems del Menú Principal ---
// Los textos están abreviados para que quepan en un LCD 16x2
MenuItem MenuMgr::menuItems[] = {
    {"1. Config WiFi",   enterWifiConfig},
    {"2. Config Reloj",  enterClockConfig},
    {"3. Cronometro",    enterCronoMode},
    {"4. Temporizador",  enterTimerMode},
    {"5. Alarmas",       enterAlarmsMenu},
    {"6. Brillo",        enterBrightnessMenu}
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

    // Lógica para el scroll del LCD (si tienes más de 2 filas)
    if (LCD_ROWS > 0) { // Evitar división por cero si no está definido
        if (currentIndex < topVisibleIndex) {
            topVisibleIndex = currentIndex;
        } else if (currentIndex >= topVisibleIndex + LCD_ROWS) {
            topVisibleIndex = currentIndex - LCD_ROWS + 1;
        }
    }
    
    draw(); // Redibujar el menú después de cada cambio
}

void MenuMgr::executeAction() {
    if (menuItems[currentIndex].action != nullptr) {
        menuItems[currentIndex].action();
    }
}

void MenuMgr::draw() {
    DisplayMgr::clearLCD();
    DisplayMgr::printLCD(0, "Menu Principal"); // Un título para el menú

    // Dibuja los ítems visibles en el LCD
    // Asumimos un LCD de 2 filas, por lo que la segunda fila muestra el item.
    // Si tuvieras 4 filas, este bucle sería más complejo.
    
    // Muestra el item seleccionado en la segunda línea
    if (currentIndex < itemsCount) {
        String line = "> " + menuItems[currentIndex].title;
        DisplayMgr::printLCD(1, line);
    }
}