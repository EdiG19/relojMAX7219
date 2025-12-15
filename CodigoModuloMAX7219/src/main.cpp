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
#include "WiFiMgr.h"

// --- DECLARACIÓN DE FUNCIONES DE ESTADO ---
void handleClockState();
void handleMenuState();
void handleCronoState();
void handleTimerState();
void handleWifiMenuState();
void handleWifiConnectingState();
void handleClockMenuState();
void handleSetTimeManualState();
void handleCronoMenuState();
void handleTimerMenuState();
void handleEditTimerState();
void handleAlarmListState();
void handleEditAlarmState();
void handleEditAlarmTimeState();
void handleEditAlarmToneState();
void handleBrightnessMenuState();
void handleEditBrightnessState();
void handleEditAlarmVolumeState();

// --- DECLARACIÓN DE FUNCIONES AUXILIARES ---
bool syncRTCToNTP();


void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);
    EepromMgr::loadSettings();
    DisplayMgr::init();
    InputMgr::init();
    MenuMgr::init();
    CronoMgr::init();
    TimerMgr::init();
    AlarmMgr::init();
    WiFiMgr::init();
    DisplayMgr::showMenuState("INICIANDO...", "HW Check");
    delay(500);
    if (!RTCMgr::init()) {
        Serial.println("Fallo RTC");
        DisplayMgr::showMenuState("ERROR HARDWARE", "Fallo RTC I2C");
        while (1);
    }
    DisplayMgr::printMatrix("LISTO");
    delay(1000);
}

void loop() {
    DisplayMgr::updateRoutine();

    switch (GlobalSettings::appState) {
        case STATE_CLOCK: handleClockState(); break;
        case STATE_MENU_MAIN: handleMenuState(); break;
        case STATE_CRONO: handleCronoState(); break;
        case STATE_TIMER: handleTimerState(); break;
        case STATE_MENU_WIFI: handleWifiMenuState(); break;
        case STATE_WIFI_CONNECTING: handleWifiConnectingState(); break;
        case STATE_MENU_CLOCK: handleClockMenuState(); break;
        case STATE_EDIT_MANUAL_TIME: handleSetTimeManualState(); break;
        case STATE_MENU_CRONO: handleCronoMenuState(); break;
        case STATE_MENU_TIMER: handleTimerMenuState(); break;
        case STATE_EDIT_TIMER: handleEditTimerState(); break;
        case STATE_MENU_ALARM_LIST: handleAlarmListState(); break;
        case STATE_EDIT_ALARM: handleEditAlarmState(); break;
        case STATE_EDIT_ALARM_TIME: handleEditAlarmTimeState(); break;
        case STATE_EDIT_ALARM_TONE: handleEditAlarmToneState(); break;
        case STATE_MENU_BRIGHTNESS: handleBrightnessMenuState(); break;
        case STATE_EDIT_BRIGHTNESS: handleEditBrightnessState(); break;
        default:
            GlobalSettings::appState = STATE_CLOCK;
            break;
    }
}

// --- ESTADOS PRINCIPALES ---

void handleClockState() {
    static unsigned long lastBrightnessUpdate = 0;

    // Actualizar brillo automático
    if (millis() - lastBrightnessUpdate > 500) {
        lastBrightnessUpdate = millis();
        if (GlobalSettings::brightnessMode == BRIGHTNESS_AUTO) {
            DisplayMgr::setMatrixBrightness(InputMgr::getAutoBrightness());
        } else if (GlobalSettings::brightnessMode == BRIGHTNESS_NIGHT) {
            DisplayMgr::setMatrixBrightness(0);
        } else { // MANUAL
            DisplayMgr::setMatrixBrightness(GlobalSettings::matrixBrightness);
        }
    }

    DisplayMgr::printLCD(0, "Modo Reloj");
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        RtcDateTime now = RTCMgr::now();
        float temp = RTCMgr::getTemperature();
        int displayHour = now.Hour();
        if (!GlobalSettings::use24hFormat) {
            if (displayHour == 0) displayHour = 12;
            else if (displayHour > 12) displayHour -= 12;
        }
        char timeBuffer[10];
        sprintf(timeBuffer, "%02d:%02d:%02d", displayHour, now.Minute(), now.Second());
        DisplayMgr::printMatrix(String(timeBuffer));
        char dateBuffer[17];
        sprintf(dateBuffer, "%02d/%02d/%4d %.1fC", now.Day(), now.Month(), now.Year(), temp);
        DisplayMgr::printLCD(1, String(dateBuffer));
        AlarmMgr::update(now.Hour(), now.Minute(), now.Second());
    }
    if (InputMgr::readButtons() == BTN_OK) {
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::init();
        MenuMgr::draw();
    }
}

void handleMenuState() {
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        MenuMgr::handleNavigation(btn);
    } else if (btn == BTN_OK) {
        MenuMgr::executeAction();
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_CLOCK;
    }
}

// --- MENÚS Y SUB-MENÚS ---

void handleBrightnessMenuState() {
    static int menuIndex = 0;
    const char* options[] = {"Modo Brillo", "Nivel Manual", "Volumen Alarma"};
    
    DisplayMgr::printLCD(0, "Menu Brillo/Sonido");
    String line = "> " + String(options[menuIndex]);

    if (menuIndex == 0) { // Modo
        if (GlobalSettings::brightnessMode == BRIGHTNESS_AUTO) line += " (Auto)";
        else if (GlobalSettings::brightnessMode == BRIGHTNESS_MANUAL) line += " (Manual)";
        else line += " (Noche)";
    } else if (menuIndex == 1) { // Nivel
        line += " (" + String(GlobalSettings::matrixBrightness) + ")";
    } else { // Volumen
        line += " (" + String(GlobalSettings::alarmVolume) + ")";
    }
    DisplayMgr::printLCD(1, line);

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        menuIndex = (menuIndex == 0) ? 2 : menuIndex - 1;
    } else if (btn == BTN_DOWN) {
        menuIndex = (menuIndex + 1) % 3;
    } else if (btn == BTN_BACK) {
        EepromMgr::saveSettings();
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        if (menuIndex == 0) { // Cambiar modo
            int currentMode = (int)GlobalSettings::brightnessMode;
            GlobalSettings::brightnessMode = (BrightnessMode)((currentMode + 1) % 3);
        } else if (menuIndex == 1) { // Ir a editar nivel
            if (GlobalSettings::brightnessMode == BRIGHTNESS_MANUAL) {
                GlobalSettings::appState = STATE_EDIT_BRIGHTNESS;
            } else {
                DisplayMgr::showMenuState("Modo no Manual", "Cambie a Manual");
                delay(1500);
            }
        }
    }
}

void handleEditBrightnessState() {
    DisplayMgr::printLCD(0, "Ajustar Brillo");
    String levelBar = "";
    for (int i = 0; i < 16; i++) {
        levelBar += (i <= GlobalSettings::matrixBrightness) ? "#" : "-";
    }
    DisplayMgr::printLCD(1, levelBar);
    
    // Aplicar en tiempo real
    DisplayMgr::setMatrixBrightness(GlobalSettings::matrixBrightness);

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        if(GlobalSettings::matrixBrightness < 15) GlobalSettings::matrixBrightness++;
    } else if (btn == BTN_DOWN) {
        if(GlobalSettings::matrixBrightness > 0) GlobalSettings::matrixBrightness--;
    } else if (btn == BTN_BACK || btn == BTN_OK) {
        GlobalSettings::appState = STATE_MENU_BRIGHTNESS;
    }
}


void handleAlarmListState() {
    static int listIndex = 0;
    DisplayMgr::printLCD(0, "Menu Alarmas");

    AlarmConfig& alarm = GlobalSettings::alarms[listIndex];
    char buffer[17];
    sprintf(buffer, "> Alarma %d %02d:%02d", listIndex + 1, alarm.hour, alarm.minute);
    String line = String(buffer);
    line += alarm.enabled ? " ON" : " OFF";
    DisplayMgr::printLCD(1, line);
    
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        listIndex = (listIndex == 0) ? 2 : listIndex - 1;
    } else if (btn == BTN_DOWN) {
        listIndex = (listIndex + 1) % 3;
    } else if (btn == BTN_BACK) {
        EepromMgr::saveSettings();
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        GlobalSettings::editingIndex = listIndex;
        GlobalSettings::appState = STATE_EDIT_ALARM;
    }
}

void handleEditAlarmState() {
    static int menuIndex = 0;
    const char* options[] = {"Ajustar Hora", "Activar/Desact.", "Tono Alarma"};

    DisplayMgr::printLCD(0, "Editando Alarma " + String(GlobalSettings::editingIndex + 1));
    String line = "> " + String(options[menuIndex]);
    
    if (menuIndex == 1) {
        line += GlobalSettings::alarms[GlobalSettings::editingIndex].enabled ? " (ON)" : " (OFF)";
    } else if (menuIndex == 2) {
        line += " (Tono " + String(GlobalSettings::alarms[GlobalSettings::editingIndex].tone + 1) + ")";
    }
    DisplayMgr::printLCD(1, line);

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        menuIndex = (menuIndex == 0) ? 2 : menuIndex -1;
    } else if (btn == BTN_DOWN) {
        menuIndex = (menuIndex + 1) % 3;
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_MENU_ALARM_LIST;
    } else if (btn == BTN_OK) {
        if (menuIndex == 0) {
            GlobalSettings::appState = STATE_EDIT_ALARM_TIME;
        } else if (menuIndex == 1) {
            AlarmMgr::toggle(GlobalSettings::editingIndex, !GlobalSettings::alarms[GlobalSettings::editingIndex].enabled);
        } else {
            GlobalSettings::appState = STATE_EDIT_ALARM_TONE;
        }
    }
}

void handleEditAlarmTimeState() {
    static uint8_t tempHour, tempMinute;
    static bool editingHour = true;
    static int currentAlarmIndex = -1;

    if (currentAlarmIndex != GlobalSettings::editingIndex) {
        currentAlarmIndex = GlobalSettings::editingIndex;
        AlarmConfig& alarm = GlobalSettings::alarms[currentAlarmIndex];
        tempHour = alarm.hour;
        tempMinute = alarm.minute;
        editingHour = true;
    }

    char timeBuffer[17];
    sprintf(timeBuffer, "Hora:    %02d:%02d", tempHour, tempMinute);
    DisplayMgr::printLCD(0, timeBuffer);

    if (editingHour) DisplayMgr::printLCD(1, "         ^^");
    else DisplayMgr::printLCD(1, "            ^^");

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        if (editingHour) tempHour = (tempHour + 1) % 24;
        else tempMinute = (tempMinute + 1) % 60;
    } else if (btn == BTN_DOWN) {
        if (editingHour) tempHour = (tempHour == 0) ? 23 : tempHour - 1;
        else tempMinute = (tempMinute == 0) ? 59 : tempMinute - 1;
    } else if (btn == BTN_BACK) {
        currentAlarmIndex = -1;
        GlobalSettings::appState = STATE_EDIT_ALARM;
    } else if (btn == BTN_OK) {
        if (editingHour) {
            editingHour = false;
        } else {
            AlarmMgr::setTime(currentAlarmIndex, tempHour, tempMinute);
            DisplayMgr::showMenuState("Hora Guardada", "");
            delay(1000);
            currentAlarmIndex = -1;
            GlobalSettings::appState = STATE_EDIT_ALARM;
        }
    }
}

void handleEditAlarmToneState() {
    AlarmConfig& alarm = GlobalSettings::alarms[GlobalSettings::editingIndex];
    
    DisplayMgr::printLCD(0, "Seleccionar Tono");
    DisplayMgr::printLCD(1, "> Tono " + String(alarm.tone + 1));

    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        int8_t newTone = alarm.tone;
        if(btn == BTN_UP) newTone++;
        else newTone--;
        if(newTone > 2) newTone = 0;
        if(newTone < 0) newTone = 2;
        alarm.tone = newTone;
        AlarmMgr::previewTone(alarm.tone);
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_EDIT_ALARM;
    } else if (btn == BTN_OK) {
        AlarmMgr::previewTone(alarm.tone);
    }
}

void handleTimerMenuState() {
    static int menuIndex = 0;
    const char* options[] = {"Iniciar Timer", "Ajustar Tiempo"};
    DisplayMgr::printLCD(0, "Menu Timer");
    String line = "> " + String(options[menuIndex]);
    if (menuIndex == 1) {
        uint32_t val = GlobalSettings::timerInitialValue;
        line += " (" + String(val / 60) + "m " + String(val % 60) + "s)";
    }
    DisplayMgr::printLCD(1, line);
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        menuIndex = 1 - menuIndex;
    } else if (btn == BTN_BACK) {
        EepromMgr::saveSettings();
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        if (menuIndex == 0) {
            TimerMgr::setInitialTime(GlobalSettings::timerInitialValue);
            TimerMgr::reset();
            GlobalSettings::appState = STATE_TIMER;
        } else {
            GlobalSettings::editingIndex = 0;
            GlobalSettings::appState = STATE_EDIT_TIMER;
        }
    }
}

void handleEditTimerState() {
    static uint8_t tempMin, tempSec;
    static bool editingMin = true;
    if (GlobalSettings::editingIndex == 0) {
        tempMin = GlobalSettings::timerInitialValue / 60;
        tempSec = GlobalSettings::timerInitialValue % 60;
        editingMin = true;
        GlobalSettings::editingIndex = 1;
    }
    char timeBuffer[17];
    sprintf(timeBuffer, "Ajuste:  %02d m %02d s", tempMin, tempSec);
    DisplayMgr::printLCD(0, timeBuffer);
    if (editingMin) {
        DisplayMgr::printLCD(1, "         ^^");
    } else {
        DisplayMgr::printLCD(1, "            ^^");
    }
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        if (editingMin) tempMin = (tempMin + 1) % 60;
        else tempSec = (tempSec + 1) % 60;
    } else if (btn == BTN_DOWN) {
        if (editingMin) tempMin = (tempMin == 0) ? 59 : tempMin - 1;
        else tempSec = (tempSec == 0) ? 59 : tempSec - 1;
    } else if (btn == BTN_BACK) {
        GlobalSettings::editingIndex = 0;
        GlobalSettings::appState = STATE_MENU_TIMER;
    } else if (btn == BTN_OK) {
        if (editingMin) {
            editingMin = false;
        } else {
            GlobalSettings::timerInitialValue = (tempMin * 60) + tempSec;
            DisplayMgr::showMenuState("Tiempo Guardado", String(tempMin) + "m " + String(tempSec) + "s");
            delay(1500);
            GlobalSettings::editingIndex = 0;
            GlobalSettings::appState = STATE_MENU_TIMER;
        }
    }
}


void handleCronoMenuState() {
    static int menuIndex = 0;
    const char* options[] = {"Iniciar Crono", "Precision"};
    DisplayMgr::printLCD(0, "Menu Cronometro");
    String line = "> " + String(options[menuIndex]);
    if (menuIndex == 1) {
        line += (GlobalSettings::cronoPrecision == PRECISION_CENTIS) ? " (0.01s)" : " (1s)";
    }
    DisplayMgr::printLCD(1, line);
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        menuIndex = 1 - menuIndex;
    } else if (btn == BTN_BACK) {
        EepromMgr::saveSettings();
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        if (menuIndex == 0) {
            CronoMgr::reset();
            GlobalSettings::appState = STATE_CRONO;
        } else {
            if (GlobalSettings::cronoPrecision == PRECISION_CENTIS) {
                GlobalSettings::cronoPrecision = PRECISION_SECONDS;
            } else {
                GlobalSettings::cronoPrecision = PRECISION_CENTIS;
            }
        }
    }
}

void handleClockMenuState() {
    static int menuIndex = 0;
    const char* options[] = {"Formato Hora", "Ajuste Manual"};
    DisplayMgr::printLCD(0, "Config. Reloj");
    String line = "> " + String(options[menuIndex]);
    if (menuIndex == 0) {
        line += (GlobalSettings::use24hFormat ? " (24h)" : " (12h)");
    }
    DisplayMgr::printLCD(1, line);
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        menuIndex = 1 - menuIndex;
    } else if (btn == BTN_BACK) {
        EepromMgr::saveSettings();
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        if (menuIndex == 0) {
            GlobalSettings::use24hFormat = !GlobalSettings::use24hFormat;
        } else {
            GlobalSettings::editingIndex = 0;
            GlobalSettings::appState = STATE_EDIT_MANUAL_TIME;
        }
    }
}

void handleSetTimeManualState() {
    static uint8_t tempHour, tempMinute;
    static bool editingHour = true;
    if (GlobalSettings::editingIndex == 0) {
        RtcDateTime now = RTCMgr::now();
        tempHour = now.Hour();
        tempMinute = now.Minute();
        editingHour = true;
        GlobalSettings::editingIndex = 1;
    }
    char timeBuffer[17];
    sprintf(timeBuffer, "Ajuste:  %02d:%02d", tempHour, tempMinute);
    DisplayMgr::printLCD(0, timeBuffer);
    if (editingHour) {
        DisplayMgr::printLCD(1, "         ^^");
    } else {
        DisplayMgr::printLCD(1, "            ^^");
    }
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP) {
        if (editingHour) tempHour = (tempHour + 1) % 24;
        else tempMinute = (tempMinute + 1) % 60;
    } else if (btn == BTN_DOWN) {
        if (editingHour) tempHour = (tempHour == 0) ? 23 : tempHour - 1;
        else tempMinute = (tempMinute == 0) ? 59 : tempMinute - 1;
    } else if (btn == BTN_BACK) {
        GlobalSettings::editingIndex = 0;
        GlobalSettings::appState = STATE_MENU_CLOCK;
    } else if (btn == BTN_OK) {
        if (editingHour) {
            editingHour = false;
        } else {
            RtcDateTime now = RTCMgr::now();
            RtcDateTime newTime(now.Year(), now.Month(), now.Day(), tempHour, tempMinute, 0);
            RTCMgr::setDateTime(newTime);
            DisplayMgr::showMenuState("Hora Guardada", newTime.format("H:i:s"));
            delay(1500);
            GlobalSettings::editingIndex = 0;
            GlobalSettings::appState = STATE_MENU_CLOCK;
        }
    }
}

void handleWifiMenuState() {
    static int wifiMenuIndex = 0;
    const char* options[] = {"Conex. Auto", "Portal Cautivo"};
    DisplayMgr::printLCD(0, "Menu WiFi");
    String line = "> " + String(options[wifiMenuIndex]);
    DisplayMgr::printLCD(1, line);
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_UP || btn == BTN_DOWN) {
        wifiMenuIndex = 1 - wifiMenuIndex;
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
    } else if (btn == BTN_OK) {
        if (wifiMenuIndex == 0) {
            GlobalSettings::appState = STATE_WIFI_CONNECTING;
        } else {
            WiFiMgr::startConfigPortal();
            if (WiFiMgr::isConnected()) {
                syncRTCToNTP();
            }
            GlobalSettings::appState = STATE_MENU_MAIN;
            MenuMgr::draw();
        }
    }
}

void handleWifiConnectingState() { 
    static unsigned long entryTime = 0;
    if (entryTime == 0) {
        entryTime = millis();
        DisplayMgr::showMenuState("Conectando WiFi", "...");
        DisplayMgr::printMatrix("WIFI");
        WiFi.begin();
    }
    if (WiFiMgr::isConnected()) {
        DisplayMgr::showMenuState("WiFi Conectado!", "Sincronizando...");
        delay(1000);
        if (syncRTCToNTP()) {
            DisplayMgr::printLCD(1, "Hora Sincronizada");
        } else {
            DisplayMgr::printLCD(1, "Fallo Sincroniz.");
        }
        delay(2000);
        entryTime = 0;
        GlobalSettings::appState = STATE_MENU_MAIN;
        MenuMgr::draw();
        return;
    }
    if (millis() - entryTime > 20000) {
        DisplayMgr::showMenuState("Fallo Conexion", "Timeout");
        WiFi.disconnect();
        delay(2000);
        entryTime = 0;
        GlobalSettings::appState = STATE_MENU_WIFI;
        return;
    }
}

bool syncRTCToNTP() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5000)) {
        Serial.println("Fallo al obtener hora de NTP");
        return false;
    }
    RtcDateTime ntpTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    RTCMgr::setDateTime(ntpTime);
    Serial.println("RTC sincronizado con hora NTP");
    return true;
}

void handleCronoState() {
    CronoMgr::update();
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_OK) {
        (CronoMgr::getState() == RUNNING) ? CronoMgr::pause() : CronoMgr::start();
    } else if (btn == BTN_DOWN) {
        CronoMgr::reset();
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_MENU_CRONO;
    }
}

void handleTimerState() {
    TimerMgr::update();
    ButtonState btn = InputMgr::readButtons();
    if (btn == BTN_OK) {
        (TimerMgr::getState() == TMR_RUNNING) ? TimerMgr::pause() : TimerMgr::start();
    } else if (btn == BTN_DOWN) {
        TimerMgr::reset();
    } else if (btn == BTN_BACK) {
        GlobalSettings::appState = STATE_MENU_TIMER;
        MenuMgr::draw();
    }
}