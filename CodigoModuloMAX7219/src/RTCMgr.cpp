#include "RTCMgr.h"

// Instanciamos el objeto pasándole la referencia a Wire (I2C)
RtcDS3231<TwoWire> RTCMgr::rtc(Wire);

bool RTCMgr::init() {
    // Iniciamos el RTC
    rtc.Begin();

    // Verificamos si el reloj está corriendo
    if (!rtc.GetIsRunning()) {
        Serial.println("RTC no estaba corriendo, iniciandolo ahora...");
        rtc.SetIsRunning(true);
    }

    // Verificamos si la hora almacenada es válida (o si se perdió la batería)
    if (!rtc.IsDateTimeValid()) {
        Serial.println("RTC perdio la confianza (Bateria baja?), actualizando...");
        
        // Si la hora no es válida, cargamos la fecha de compilación
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        rtc.SetDateTime(compiled);
    }

    // Verificamos que el chip esté conectado realmente (Wire check simple)
    Wire.beginTransmission(0x68); // Dirección I2C del DS3231
    if (Wire.endTransmission() != 0) {
        return false; // No se encontró el dispositivo
    }

    return true;
}

RtcDateTime RTCMgr::now() {
    return rtc.GetDateTime();
}

void RTCMgr::adjust(RtcDateTime dt) {
    rtc.SetDateTime(dt);
    // Verificar y reiniciar estado si es necesario
    if (!rtc.GetIsRunning()) rtc.SetIsRunning(true);
}

float RTCMgr::getTemperature() {
    RtcTemperature temp = rtc.GetTemperature();
    return temp.AsFloatDegC();
}