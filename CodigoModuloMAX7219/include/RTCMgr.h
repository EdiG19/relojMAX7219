#ifndef RTCMGR_H
#define RTCMGR_H

#include <Arduino.h>
#include <Wire.h>
#include <RtcDS3231.h> // Librería de Makuna para el DS3231

class RTCMgr {
public:
    // Inicia el módulo. Devuelve true si la conexión es exitosa.
    static bool init();

    // Devuelve el objeto de fecha/hora propio de la librería Makuna
    static RtcDateTime now();

    // Ajusta la hora del módulo
    static void adjust(RtcDateTime dt);

    // Devuelve la temperatura del chip DS3231
    static float getTemperature();

private:
    // Definimos el objeto RTC usando la interfaz TwoWire (I2C estándar)
    static RtcDS3231<TwoWire> rtc;
};

#endif