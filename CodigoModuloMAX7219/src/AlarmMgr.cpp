#include "AlarmMgr.h"
#include "GlobalSettings.h"

// Definiciones de PWM para el ESP32
#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION 8
#define LEDC_BASE_FREQ 1000

bool AlarmMgr::isCurrentlySounding = false;
unsigned long AlarmMgr::alarmStartTime = 0;

void AlarmMgr::init() {
    ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, LEDC_CHANNEL);
    stopTone();
}

bool AlarmMgr::isSounding() {
    return isCurrentlySounding;
}

void AlarmMgr::playTone(int toneIndex) {
    if (isCurrentlySounding) return;

    int freq = 440; // Tono por defecto (La)
    if (toneIndex == 1) freq = 523; // Do
    else if (toneIndex == 2) freq = 659; // Mi
    // Se podrían añadir más tonos aquí

    int dutyCycle = map(GlobalSettings::alarmVolume, 0, 10, 0, 50);
    
    ledcWriteTone(LEDC_CHANNEL, freq);
    ledcWrite(LEDC_CHANNEL, dutyCycle);
    
    isCurrentlySounding = true;
    alarmStartTime = millis();
}

void AlarmMgr::stopTone() {
    ledcWrite(LEDC_CHANNEL, 0);
    isCurrentlySounding = false;
}

void AlarmMgr::update(int currentHour, int currentMinute, int currentSecond) {
    if (isCurrentlySounding) {
        if (millis() - alarmStartTime >= 5000) { // Detener después de 5s
            stopTone();
        }
        return;
    }
    
    // Para no sobrecargar, solo chequear al inicio de un minuto
    if (currentSecond != 0) return;

    for (int i = 0; i < 3; i++) {
        if (GlobalSettings::alarms[i].enabled &&
            GlobalSettings::alarms[i].hour == currentHour && 
            GlobalSettings::alarms[i].minute == currentMinute) {
            
            playTone(GlobalSettings::alarms[i].tone); // <-- USA EL TONO DE LA ALARMA
            return; // Salir para no activar múltiples alarmas a la vez
        }
    }
}

// Función para módulos externos (Timer), usa un tono genérico (0)
void AlarmMgr::soundAlarm() {
    playTone(0);
}

void AlarmMgr::setTime(int alarmIndex, int hour, int minute) {
    if (alarmIndex >= 0 && alarmIndex < 3) {
        GlobalSettings::alarms[alarmIndex].hour = hour;
        GlobalSettings::alarms[alarmIndex].minute = minute;
    }
}

void AlarmMgr::setTone(int alarmIndex, int toneIndex) {
    if (alarmIndex >= 0 && alarmIndex < 3) {
        GlobalSettings::alarms[alarmIndex].tone = constrain(toneIndex, 0, 2);
    }
}

void AlarmMgr::toggle(int alarmIndex, bool enabled) {
    if (alarmIndex >= 0 && alarmIndex < 3) {
        GlobalSettings::alarms[alarmIndex].enabled = enabled;
    }
}

void AlarmMgr::setVolume(int volume) {
    GlobalSettings::alarmVolume = constrain(volume, 0, 10);
}

void AlarmMgr::previewTone(int toneIndex) {
    playTone(toneIndex);
    delay(500);
    stopTone();
}

void AlarmMgr::stop() {
    stopTone();
}