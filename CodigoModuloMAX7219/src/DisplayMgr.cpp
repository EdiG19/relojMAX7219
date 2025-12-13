// DisplayMgr.cpp
#include "DisplayMgr.h"

// Instanciación
LiquidCrystal_I2C DisplayMgr::lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
MD_Parola DisplayMgr::matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void DisplayMgr::init() {
    // LCD
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    
    // Matriz
    matrix.begin();
    matrix.setIntensity(1); // 0 a 15
    matrix.setTextAlignment(PA_CENTER);
}

void DisplayMgr::clearLCD() {
    lcd.clear();
}

void DisplayMgr::printLCD(int row, String text) {
    if (row >= 0 && row < LCD_ROWS) {
        lcd.setCursor(0, row);
        // Relleno de espacios para borrar residuos anteriores
        lcd.print("                "); 
        lcd.setCursor(0, row);
        if (text.length() > LCD_COLS) text = text.substring(0, LCD_COLS);
        lcd.print(text);
    }
}

void DisplayMgr::showMenuState(String line1, String line2) {
    printLCD(0, line1);
    printLCD(1, line2);
}

void DisplayMgr::setMatrixBrightness(int intensity) {
    if (intensity < 0) intensity = 0;
    if (intensity > 15) intensity = 15;
    matrix.setIntensity(intensity);
}

void DisplayMgr::printMatrix(String text) {
    // Convertimos String a char array para Parola
    matrix.setTextAlignment(PA_CENTER);
    matrix.print(text.c_str());
}

void DisplayMgr::updateRoutine() {
    matrix.displayAnimate();
}