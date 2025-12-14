#include <Arduino.h>
#include "GlobalSettings.h"
#include "DisplayMgr.h"
#include "InputMgr.h"
#include "WiFiMgr.h"
#include "CronoMgr.h"
#include "AlarmMgr.h"
#include "TimerMgr.h"

// --- ESTADOS DEL SISTEMA ---
enum SystemMode {
    MODE_CLOCK,
    MODE_TIMER,
    MODE_ALARM_MENU,
    MODE_BRIGHTNESS_MENU
    // ... otros modos principales
};

enum TimerMenuState {
    TIMER_SHOW,         // Muestra la cuenta regresiva
    TIMER_SET_PRECISION, // Menú para cambiar la precisión
    TIMER_SET_TIME      // Menú para ajustar el tiempo
};

// --- VARIABLES GLOBALES DE ESTADO ---
SystemMode currentMode = MODE_TIMER; // Iniciar en modo Temporizador
TimerMenuState timerMenuState = TIMER_SHOW;

// --- INSTANCIAS DE MANEJADORES ---
TimerMgr timerMgr;

// --- VARIABLES PARA EL MENÚ DEL TEMPORIZADOR ---
int selectedTimePreset = 1; // 0=10s, 1=30s, 2=1m, 3=5m
const int timePresets[] = {10, 30, 60, 300}; // en segundos
bool menuNeedsRedraw = true;


// --- DECLARACIÓN DE FUNCIONES DE MENÚ ---
void handleTimerMode();
void drawTimerMenu();


void setup() {
    Serial.begin(115200);
    
    // Inicializar Módulos
    GlobalSettings::init();
    DisplayMgr::init();
    InputMgr::init();
    AlarmMgr::init();
    timerMgr.init();
    // WiFiMgr::init(); // No lo usamos por ahora
    // CronoMgr::init();

    // Configuración inicial del temporizador
    timerMgr.setInitialTime(timePresets[selectedTimePreset]);

    DisplayMgr::printLCD(0, "Modo Temporizador");
    delay(1000);
    menuNeedsRedraw = true;
}

void loop() {
    // 1. Actualizar módulos que lo necesiten en cada ciclo
    DisplayMgr::updateRoutine();
    InputMgr::update();

    // 2. Manejar el modo actual del sistema
    switch (currentMode) {
        case MODE_TIMER:
            handleTimerMode();
            break;
        case MODE_CLOCK:
            // Lógica del modo reloj...
            break;
        // otros modos...
    }

    // 3. Actualizar el temporizador si no estamos en un submenú
    if (currentMode == MODE_TIMER && timerMenuState == TIMER_SHOW) {
        timerMgr.update();
    }
}

void handleTimerMode() {
    ButtonState btn = InputMgr::readButtons();

    if (btn != BTN_NONE) {
        menuNeedsRedraw = true; // Un botón se ha pulsado, redibujar menú
    }

    switch (timerMenuState) {
        case TIMER_SHOW:
            // Lógica de botones para la pantalla principal del timer
            if (btn == BTN_OK) {
                if (timerMgr.getState() == TMR_RUNNING) timerMgr.pause();
                else if (timerMgr.getState() == TMR_PAUSED || timerMgr.getState() == TMR_STOPPED) timerMgr.start();
                else if (timerMgr.getState() == TMR_EXPIRED) timerMgr.reset();
            } else if (btn == BTN_NEXT) {
                // Cambiar a modo CLOCK (como ejemplo)
                currentMode = MODE_CLOCK;
                DisplayMgr::printLCD(0, "Modo Reloj"); // Placeholder
                delay(1000);
                menuNeedsRedraw = true;
                return; // Salir para no procesar más en este ciclo
            } else if (btn == BTN_PREV) {
                timerMgr.reset();
            } else if (InputMgr::isLongPress(BTN_OK)) {
                 timerMenuState = TIMER_SET_TIME; // Entrar a menú de tiempo
            } else if(InputMgr::isLongPress(BTN_NEXT)){
                 timerMenuState = TIMER_SET_PRECISION;
            }
            break;

        case TIMER_SET_TIME:
            if (btn == BTN_NEXT) {
                selectedTimePreset = (selectedTimePreset + 1) % 4;
            } else if (btn == BTN_PREV) {
                selectedTimePreset = (selectedTimePreset + 3) % 4;
            } else if (btn == BTN_OK) {
                timerMgr.setInitialTime(timePresets[selectedTimePreset]);
                timerMenuState = TIMER_SHOW;
            }
            break;

        case TIMER_SET_PRECISION:
            if (btn == BTN_NEXT || btn == BTN_PREV) {
                TimerPrecision current = timerMgr.getPrecision();
                timerMgr.setPrecision(current == PRECISION_HMS ? PRECISION_MS_CS : PRECISION_HMS);
            } else if (btn == BTN_OK) {
                timerMenuState = TIMER_SHOW;
            }
            break;
    }

    if (menuNeedsRedraw) {
        drawTimerMenu();
        menuNeedsRedraw = false;
    }
}

void drawTimerMenu() {
    DisplayMgr::clearLCD();
    switch (timerMenuState) {
        case TIMER_SHOW:
            {
                String line1 = "Timer ";
                if (timerMgr.getState() == TMR_RUNNING) line1 += "[RUN]";
                else if (timerMgr.getState() == TMR_PAUSED) line1 += "[PAUSA]";
                else line1 += "[STOP]";
                DisplayMgr::printLCD(0, line1);
                DisplayMgr::printLCD(1, "OK:Start/Siguiente");
            }
            break;
        case TIMER_SET_TIME:
            DisplayMgr::printLCD(0, "Establecer Tiempo");
            char buffer[10];
            sprintf(buffer, "%d s", timePresets[selectedTimePreset]);
            DisplayMgr::printLCD(1, String(buffer));
            break;
        case TIMER_SET_PRECISION:
            DisplayMgr::printLCD(0, "Precision");
            String precisionStr = timerMgr.getPrecision() == PRECISION_HMS ? "HH:MM:SS" : "MM:SS:CS";
            DisplayMgr::printLCD(1, precisionStr);
            break;
    }
}

