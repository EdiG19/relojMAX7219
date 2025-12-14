#include "AlarmMgr.h"
#include "GlobalSettings.h"

// Definiciones de PWM para el ESP32
#define LEDC_CHANNEL 0
#define LEDC_RESOLUTION 8 // 8 bits de resolución (0-255)
#define LEDC_BASE_FREQ 1000 // 1000 Hz

bool AlarmMgr::isCurrentlySounding = false;
unsigned long AlarmMgr::alarmStartTime = 0;

bool AlarmMgr::isSounding() {
    return isCurrentlySounding;
}

void AlarmMgr::init() {
    // Configura el canal PWM para el Buzzer
    ledcSetup(LEDC_CHANNEL, LEDC_BASE_FREQ, LEDC_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, LEDC_CHANNEL);
    
    // Lo apagamos inicialmente
    stopTone();
}

void AlarmMgr::playTone(int toneIndex) {
    if (isCurrentlySounding) return; // Si ya está sonando, no hacemos nada más

    // 1. Obtener frecuencia basada en el índice
    int freq = TONE_FREQ_1;
    if (toneIndex == 1) freq = TONE_FREQ_2;
    else if (toneIndex == 2) freq = TONE_FREQ_3;

    // 2. Obtener Volumen (GlobalSettings::alarmVolume está en 0-10, lo mapeamos a 0-50)
    int dutyCycle = map(GlobalSettings::alarmVolume, 0, 10, 0, 50);
    
    // 3. Reproducir (ledcWriteTone ajusta la frecuencia automáticamente)
    ledcWriteTone(LEDC_CHANNEL, freq);
    ledcWrite(LEDC_CHANNEL, dutyCycle);
    
    isCurrentlySounding = true;
    alarmStartTime = millis(); // Guardamos cuándo empezó a sonar
}

void AlarmMgr::stopTone() {
    ledcWrite(LEDC_CHANNEL, 0); // Duty Cycle a cero (silencio)
    isCurrentlySounding = false;
}

// Lógica principal: Verifica si debe sonar alguna alarma O si debe detenerse
void AlarmMgr::update(int currentHour, int currentMinute, int currentSecond) {
    // Si la alarma está sonando, verificar si debe detenerse después de 5 segundos
    if (isCurrentlySounding) {
        if (millis() - alarmStartTime >= 5000) {
            stop();
        }
        return; // No hacer nada más si la alarma está activa
    }
    
    // La alarma solo suena si el segundo es 00
    if (currentSecond != 0) return;

    // Recorremos las 3 alarmas
    for (int i = 0; i < 3; i++) {
        if (GlobalSettings::alarms[i].enabled) {
            
            // Si coincide la hora y el minuto
            if (GlobalSettings::alarms[i].hour == currentHour && 
                GlobalSettings::alarms[i].minute == currentMinute) {
                
                playTone(GlobalSettings::alarmToneIndex); // SUENA LA ALARMA
                
                // Si es la alarma 0 (la de prueba de 10s), la desactivamos para que no se repita
                if (i == 0) {
                    GlobalSettings::alarms[i].enabled = false; 
                }
                return;
            }
        }
    }
}

void AlarmMgr::setTime(int alarmIndex, int hour, int minute) {
    if (alarmIndex >= 0 && alarmIndex < 3) {
        GlobalSettings::alarms[alarmIndex].hour = hour;
        GlobalSettings::alarms[alarmIndex].minute = minute;
    }
}

void AlarmMgr::toggle(int alarmIndex, bool enabled) {
    if (alarmIndex >= 0 && alarmIndex < 3) {
        GlobalSettings::alarms[alarmIndex].enabled = enabled;
    }
}

void AlarmMgr::setVolume(int volume) {
    // Clamp/constrain the value between 0 and 10
    GlobalSettings::alarmVolume = constrain(volume, 0, 10);
}

void AlarmMgr::selectRingtone(int toneIndex) {
    // Clamp/constrain the value between 0 and 2
    GlobalSettings::alarmToneIndex = constrain(toneIndex, 0, 2);
}

void AlarmMgr::previewTone(int toneIndex) {
    playTone(toneIndex);
    delay(500); // Toca el tono por medio segundo
    stopTone();
}

void AlarmMgr::stop() {
    stopTone();
}