#include "MenuMgr.h"
#include "GlobalSettings.h"
#include "CronoMgr.h"
#include "TimerMgr.h"
#include "WiFiMgr.h"
#include "AlarmMgr.h"

// Variable externa que controla el modo del sistema
extern int currentSystemMode; 

// Definición de modos (deben coincidir con el main)
#define MODE_CLOCK 0
#define MODE_CRONO 1
#define MODE_TIMER 2
#define MODE_MENU  3

// ==========================================
// 1. CALLBACKS (Funciones de las opciones)
// ==========================================

void cb_GoToClock() {
    currentSystemMode = MODE_CLOCK;
    DisplayMgr::clearLCD();
}

void cb_GoToCrono() {
    currentSystemMode = MODE_CRONO;
    CronoMgr::reset();
}

void cb_GoToTimer() {
    currentSystemMode = MODE_TIMER;
    TimerMgr::reset();
}

void cb_ToggleFormat() {
    GlobalSettings::use24hFormat = !GlobalSettings::use24hFormat;
    DisplayMgr::showMenuState("FORMATO", GlobalSettings::use24hFormat ? "24 Horas" : "12 Horas");
    delay(1000);
    MenuMgr::draw();
}

void cb_ToggleNightMode() {
    if (GlobalSettings::brightnessMode == BRIGHTNESS_AUTO) {
        GlobalSettings::brightnessMode = BRIGHTNESS_MANUAL;
        DisplayMgr::showMenuState("BRILLO", "Manual (Fijo)");
    } else if (GlobalSettings::brightnessMode == BRIGHTNESS_MANUAL) {
        GlobalSettings::brightnessMode = BRIGHTNESS_NIGHT;
        DisplayMgr::showMenuState("BRILLO", "Noche (OFF)");
    } else {
        GlobalSettings::brightnessMode = BRIGHTNESS_AUTO;
        DisplayMgr::showMenuState("BRILLO", "Automatico");
    }
    delay(1000);
    MenuMgr::draw();
}

void cb_ConfigWiFi() {
    WiFiMgr::startConfigPortal();
    MenuMgr::draw();
}

// ==========================================
// 2. DEFINICIÓN DEL ARRAY (Aquí arreglamos el error)
// ==========================================

// PRIMERO: Definimos la lista completa. 
// Al poner los datos entre {}, el compilador ya sabe el tamaño.
MenuItem MenuMgr::menuItems[] = {
    { "Volver Reloj",   cb_GoToClock },
    { "Cronometro",     cb_GoToCrono },
    { "Temporizador",   cb_GoToTimer },
    { "Formato 12/24h", cb_ToggleFormat },
    { "Modo Brillo",    cb_ToggleNightMode },
    { "Config WiFi",    cb_ConfigWiFi }
};

// SEGUNDO: Ahora sí podemos calcular el tamaño (sizeof)
// Si pusieras esto antes del array, daría el error de "tipo incompleto".
const int MenuMgr::itemsCount = sizeof(MenuMgr::menuItems) / sizeof(MenuItem);


// ==========================================
// 3. RESTO DE LA LÓGICA
// ==========================================

int MenuMgr::currentIndex = 0;
int MenuMgr::topVisibleIndex = 0;

void MenuMgr::init() {
    currentIndex = 0;
    topVisibleIndex = 0;
}

void MenuMgr::handleNavigation(ButtonState btn) {
    if (btn == BTN_DOWN) {
        currentIndex++;
        if (currentIndex >= itemsCount) currentIndex = 0;
    } 
    else if (btn == BTN_UP) {
        currentIndex--;
        if (currentIndex < 0) currentIndex = itemsCount - 1;
    }

    // Scroll simple
    if (currentIndex < topVisibleIndex) {
        topVisibleIndex = currentIndex;
    } else if (currentIndex >= topVisibleIndex + 2) {
        topVisibleIndex = currentIndex - 1;
    }

    draw();
}

void MenuMgr::executeAction() {
    if (menuItems[currentIndex].action != NULL) {
        menuItems[currentIndex].action(); 
    }
}

void MenuMgr::draw() {
    for (int row = 0; row < 2; row++) {
        int itemIndex = topVisibleIndex + row;
        
        if (itemIndex < itemsCount) {
            String text = menuItems[itemIndex].title;
            
            if (itemIndex == currentIndex) {
                DisplayMgr::printLCD(row, ">" + text);
            } else {
                DisplayMgr::printLCD(row, " " + text);
            }
        } else {
            DisplayMgr::printLCD(row, ""); 
        }
    }
}