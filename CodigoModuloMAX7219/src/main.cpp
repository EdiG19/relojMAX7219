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

    // 2. Aplicar el brillo a la pantalla
    // El MAX7219 acepta 0-15. Tu función getAutoBrightness ya devuelve 0-15.
    for(int i=0; i<4; i++) {
        display.setIntensity(i, brilloCalculado);
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

    delay(200); // Refresco rápido para notar cambios al tapar el sensor
}