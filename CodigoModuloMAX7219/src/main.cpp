#include <Arduino.h>
#include "GlobalSettings.h"
#include "InputMgr.h" // Ahora incluimos este fichero que trae PIN_LDR

// --- CONFIGURACIÓN DE PANTALLA ---
// Asegúrate de tener la librería "LedControl" instalada o cambia por la tuya.
#include "LedControl.h" 

// Pines SPI para el MAX7219 (Ajusta si son diferentes en tu PCB)
const int DATA_PIN = 23; // MOSI habitual en ESP32
const int CLK_PIN  = 18; // SCK habitual en ESP32
const int CS_PIN   = 5;  // SS habitual en ESP32

// Inicializamos para 4 módulos en cascada (típico en relojes)
LedControl display = LedControl(DATA_PIN, CLK_PIN, CS_PIN, 4); 

void setup() {
    Serial.begin(115200);
    Serial.println("\n--- TEST DE BRILLO AUTOMÁTICO (GPIO 32) ---");

    // 1. Inicialización de tus gestores
    // InputMgr::init() configura el PIN_LDR (32) internamente.
    InputMgr::init(); 
    
    // GlobalSettings no es estrictamente necesario para este test aislado,
    // pero lo iniciamos por buena práctica.
    GlobalSettings::init(); 

    // 2. Inicialización del Hardware MAX7219
    for(int i=0; i<4; i++) {
        display.shutdown(i, false); // Despertar pantalla
        display.clearDisplay(i);    // Limpiar basura
        display.setIntensity(i, 1); // Empezar bajito
    }

    // Dibujamos algo para ver el cambio de brillo (ej. "LUZ")
    // (Esto es simplificado, depende de tu librería de fuentes)
    display.setChar(0, 3, 'L', false);
    display.setChar(0, 2, 'U', false);
    display.setChar(0, 1, 'Z', false);
}

void loop() {
    // 1. Obtener el brillo calculado
    // Llama a tu función que ya tiene la curva gamma y los límites (30-1200)
    int brilloCalculado = InputMgr::getAutoBrightness();

<<<<<<< HEAD
    // 2. Aplicar el brillo a la pantalla
    // El MAX7219 acepta 0-15. Tu función getAutoBrightness ya devuelve 0-15.
    for(int i=0; i<4; i++) {
        display.setIntensity(i, brilloCalculado);
=======
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
        case STATE_EDIT_ALARM_VOLUME: handleEditAlarmVolumeState(); break;
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
>>>>>>> 646555413e0a63ad482b81c74405e95396c979ba
    }

    // 3. Depuración Serial (Monitor Serie)
    // Usamos PIN_LDR directamente porque está definido en InputMgr.h
    int valorCrudo = analogRead(PIN_LDR); 
    
    Serial.print("Sensor en Pin ");
    Serial.print(PIN_LDR);
    Serial.print(" | Valor Crudo: ");
    Serial.print(valorCrudo);
    Serial.print(" | Brillo Calculado (0-15): ");
    Serial.println(brilloCalculado);

<<<<<<< HEAD
    delay(200); // Refresco rápido para notar cambios al tapar el sensor
=======
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
            RTCMgr::adjust(newTime);
            //DisplayMgr::showMenuState("Hora Guardada", newTime.format("H:i:s"));
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
    RTCMgr::adjust(ntpTime);
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
>>>>>>> 646555413e0a63ad482b81c74405e95396c979ba
}