#include <Arduino.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "time.h"

// --- 1. CONFIGURACIÓN DE PANTALLA (HARDWARE) ---
// Si las letras salen al revés o desordenadas, cambia GENERIC_HW por FC16_HW
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
//#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define MAX_DEVICES   4             // Número de módulos
#define CS_PIN        27            // Pin CS definido por ti

// Pines SPI por defecto en ESP32: CLK=18, MOSI=23. No es necesario definirlos si usas VSPI default.
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// --- 2. CONFIGURACIÓN DE HORA (NTP) ---
const char* ntpServer = "pool.ntp.org";
// Ajuste de zona horaria en SEGUNDOS.
// Ejemplo: UTC -5 (Colombia, Perú, Panamá, Ecuador) = -5 * 3600 = -18000
const long  gmtOffset_sec = -18000; 
const int   daylightOffset_sec = 0; // Cambiar a 3600 si hay horario de verano

// Variables para el control de tiempo
char timeString[9]; // Buffer para guardar "HH:MM" o "HH:MM:SS"
unsigned long lastTimeUpdate = 0;

void setup() {
  Serial.begin(115200);

  // A. INICIAR PANTALLA
  P.begin();
  P.setIntensity(2); // Brillo (0-15)
  P.setTextAlignment(PA_CENTER); // Centrar texto
  P.print("ESPERA..."); // Mensaje inicial antes de conectar

  // B. INICIAR WIFI MANAGER
  WiFiManager wm;
  
  // Si quieres resetear las credenciales guardadas para probar, descomenta esto:
  // wm.resetSettings();

  // Intenta conectar. Si falla, crea un AP llamado "RelojAP" sin contraseña
  // El código se queda BLOQUEADO aquí hasta que se conecte a WiFi
  bool res = wm.autoConnect("RelojAP"); 

  if(!res) {
    Serial.println("Fallo al conectar");
    P.print("ERROR WF");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("Conectado al WiFi :)");
    P.print("WIFI OK");
    delay(1000); // Pequeña pausa para ver que conectó
  }

  // C. INICIAR CLIENTE DE HORA (NTP)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // Ejecutar animación de Parola (necesario para refrescar la pantalla)
  if (P.displayAnimate()) {
    
    // Actualizar la hora cada 1000ms (1 segundo) para no saturar
    if (millis() - lastTimeUpdate > 1000) {
      lastTimeUpdate = millis();
      updateTime();
    }
  }
}

// Función auxiliar para obtener y formatear la hora
void updateTime() {
  struct tm timeinfo;
  
  // Intenta obtener la hora. Si falla (aún no sincroniza), sale de la función
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obteniendo la hora");
    return; // Mantiene el mensaje anterior o espera
  }

  // Formatear la hora en el buffer timeString
  // %H = Hora (00-23), %M = Minuto, %S = Segundo
  // Si quieres segundos usa: strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);
  strftime(timeString, sizeof(timeString), "%H:%M", &timeinfo);
  
  // Mostrar en la matriz
  P.setTextAlignment(PA_CENTER);
  P.print(timeString);
}