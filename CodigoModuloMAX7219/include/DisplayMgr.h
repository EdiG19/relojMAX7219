#ifndef DISPLAYMGR_H
#define DISPLAYMGR_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// --- CONFIGURACIÓN DE HARDWARE ---
// LCD
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// MATRIZ MAX7219
// Nota: Usamos VSPI por defecto (CLK=18, MOSI=23 en ESP32)
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES   4
#define CS_PIN        27

// Clase estática o Namespace para manejar las pantallas globalmente
class DisplayMgr {
public:
    // Inicialización
    static void init();

    // --- Funciones para LCD (Menú) ---
    static void clearLCD();
    static void printLCD(int row, String text);
    static void showMenuState(String line1, String line2); // Útil para título y opción

    // --- Funciones para Matriz (Hora/Crono) ---
    static void setMatrixBrightness(int intensity); // 0-15
    static void setLcdBacklight(bool on);
    static void printMatrix(String text);
    static void updateRoutine(); // Llamar en el loop() para animaciones de Parola

private:
    // Objetos internos (no accesibles desde fuera)
    static LiquidCrystal_I2C lcd;
    static MD_Parola matrix;
};

#endif