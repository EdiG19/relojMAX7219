#include <Arduino.h>
#include <EEPROM.h>
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "RTCMgr.h"
#include "MenuMgr.h"
#include "CronoMgr.h"
#include "TimerMgr.h"
#include "AlarmMgr.h"
#include "EepromMgr.h"

// --- DECLARACIÓN DE FUNCIONES DE ESTADO ---
void handleClockState();
void handleMenuState();
void handleCronoState();
void handleTimerState();
void handleAlarmMenuState();
void handleConfigMenuState();


void setup() {
    Serial.begin(115200);

    // 1. Inicializar EEPROM y cargar configuraciones
    EEPROM.begin(EEPROM_SIZE);
    EepromMgr::loadSettings(); // Esto carga desde EEPROM o usa defaults si no hay nada

    // 2. Inicializaciones del resto de módulos (hardware y estados volátiles)
    DisplayMgr::init();
    InputMgr::init();
    MenuMgr::init();
    CronoMgr::init();
    TimerMgr::init();
    AlarmMgr::init();

    DisplayMgr::showMenuState("INICIANDO...", "HW Check");
    delay(500);

    if (!RTCMgr::init()) {
        Serial.println("Fallo RTC");
        DisplayMgr::showMenuState("ERROR HARDWARE", "Fallo RTC I2C");
        while (1); // Bloqueo si el RTC no funciona
    }
    
    DisplayMgr::printMatrix("LISTO");
    delay(1000);
}

void loop() {
    // 1. Rutina de actualización OBLIGATORIA para la matriz
    DisplayMgr::updateRoutine();

    // 2. Selección del modo de la aplicación (Máquina de estados)
    switch (GlobalSettings::appState) {
        case STATE_CLOCK:
            handleClockState();
            break;
        case STATE_MENU_MAIN:
            handleMenuState();
            break;
        case STATE_CRONO:
            handleCronoState();
            break;
        case STATE_TIMER:
            handleTimerState();
            break;
        case STATE_MENU_ALARM:
            handleAlarmMenuState();
            break;
        case STATE_MENU_CONFIG:
            handleConfigMenuState();
            break;
    }
}

// --- IMPLEMENTACIÓN DE LAS FUNCIONES DE ESTADO ---

void handleClockState() {
    // Título en el LCD
    DisplayMgr::printLCD(0, "Modo Reloj");

    // Lógica de actualización cada 1 segundo
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();

        RtcDateTime now = RTCMgr::now();
        float temp = RTCMgr::getTemperature();

        // Formatear y mostrar hora en la Matriz
        char timeBuffer[10];
        sprintf(timeBuffer, "%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());
        DisplayMgr::printMatrix(String(timeBuffer));

        // Formatear y mostrar fecha/temp en el LCD
        char dateBuffer[17];
        sprintf(dateBuffer, "%02d/%02d/%4d %.1fC", now.Day(), now.Month(), now.Year(), temp);
        DisplayMgr::printLCD(1, String(dateBuffer));
        
        // Comprobar si alguna alarma debe sonar
        AlarmMgr::update(now.Hour(), now.Minute(), now.Second());
    }

    // Comprobar si se pulsa un botón para entrar al menú
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_OK) {
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::init(); // Reseteamos la posición del menú
        MenuMgr::draw(); // Dibujamos el menú al entrar
    }
}


void handleMenuState() {
    // La lógica de dibujar el menú ya la hace MenuMgr::draw()
    // Aquí solo gestionamos la navegación
    ButtonState btn = InputMgr::readButtons();

    if (btn == BTN_UP || btn == BTN_DOWN) {
        MenuMgr::handleNavigation(btn);
    } else if (btn == BTN_OK) {
        MenuMgr::executeAction(); // Esto cambia el estado a Crono, Timer, etc.
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_CLOCK;
    }
}


void handleCronoState() {
    // 1. Dejar que el manager del cronómetro haga su lógica
    CronoMgr::update();

    // 2. Gestionar la entrada de usuario
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_OK) {
        if (CronoMgr::getState() == RUNNING) {
            CronoMgr::pause();
        }
        else {
            CronoMgr::start();
        }
    } else if (btn == BTN_DOWN) {
        CronoMgr::reset();
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw(); // Volver a dibujar el menú
    }
}


void handleTimerState() {
    // 1. Dejar que el manager del temporizador haga su lógica
    TimerMgr::update();

    // 2. Gestionar la entrada de usuario
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_OK) {
        if (TimerMgr::getState() == TMR_RUNNING) {
            TimerMgr::pause();
        }
        else {
            TimerMgr::start();
        }
    } else if (btn == BTN_DOWN) {
        TimerMgr::reset();
    } else if (btn == BTN_BACK) {
        // Por ahora, no hay submenú de configuración de tiempo.
        // En un futuro, BTN_UP podría incrementar minutos, BTN_DOWN decrementar, etc.
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    }
}

// ---- Modos de Menú (simples por ahora) ----

void handleAlarmMenuState() {
    DisplayMgr::showMenuState("Sub-Menu Alarmas", "UP/DOWN: Sel", "OK: Edit", "BACK: Salir");

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_BACK) {
        EepromMgr::saveSettings(); // Guardamos cambios al salir
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    }
    // Aquí iría la lógica para editar alarmas
}

void handleConfigMenuState() {
    DisplayMgr::showMenuState("Sub-Menu Config", "Brillo, WiFi...", "...", "BACK: Salir");

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_BACK) {
        EepromMgr::saveSettings(); // Guardamos cambios al salir
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    }
    // Aquí iría la lógica para cambiar settings
}