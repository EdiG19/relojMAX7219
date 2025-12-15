// DisplayMgr.cpp
#include "DisplayMgr.h"

// Instanciación de objetos
LiquidCrystal_I2C DisplayMgr::lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
MD_Parola DisplayMgr::matrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Instanciación del buffer de LCD
String DisplayMgr::lcd_buffer[LCD_ROWS];

void DisplayMgr::init() {
    // LCD
    pinMode(LCD_PWM_PIN,OUTPUT);
    analogWrite(LCD_PWM_PIN, 200);
    Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    for (int i = 0; i < LCD_ROWS; i++) {
        lcd_buffer[i] = "";
    }
    
    // Matriz
    matrix.begin();
    matrix.setIntensity(1); // 0 a 15
    matrix.setTextAlignment(PA_CENTER);
}

void DisplayMgr::clearLCD() {
    lcd.clear();
    for (int i = 0; i < LCD_ROWS; i++) {
        lcd_buffer[i] = "";
    }
}

void DisplayMgr::printLCD(int row, String text) {
    if (row < 0 || row >= LCD_ROWS) return;

    // Truncar texto si es más largo que la pantalla
    if (text.length() > LCD_COLS) {
        text = text.substring(0, LCD_COLS);
    }

    // Comparar con el buffer para evitar escrituras innecesarias
    if (text == lcd_buffer[row]) {
        return; // No hay nada que hacer, el texto es el mismo
    }

    // Si el texto nuevo es más corto, hay que rellenar con espacios para borrar
    int oldLen = lcd_buffer[row].length();
    int newLen = text.length();
    if (newLen < oldLen) {
        String filler = "";
        for (int i = 0; i < oldLen - newLen; i++) {
            filler += " ";
        }
        text += filler;
    }
    
    // Escribir en el LCD y actualizar el buffer
    lcd.setCursor(0, row);
    lcd.print(text);
    lcd_buffer[row] = text; // Guardar el texto real, sin relleno
    // Nota: Guardamos el texto original para que la próxima comparación sea correcta.
    // Si el texto nuevo era 'A' y el viejo 'ABC', imprimimos 'A  ' pero guardamos 'A' en el buffer.
    // Para simplificar, guardaremos el texto impreso, y la próxima vez se re-imprimirá si el nuevo es más corto.
    // La lógica actual es más simple y previene el parpadeo de re-escritura constante.
    if (text.length() > newLen) { // Si hubo relleno
         lcd_buffer[row] = text.substring(0, newLen);
    } else {
         lcd_buffer[row] = text;
    }

}

void DisplayMgr::showMenuState(String line1, String line2) {
    printLCD(0, line1);
    printLCD(1, line2);
}

void DisplayMgr::setMatrixBrightness(int intensity) {
    if (intensity < 3) intensity = 0;
    if (intensity > 11) intensity = 15;
    matrix.setIntensity(intensity);

    // 3. Aplicar brillo al LCD (PWM 0-255)
    // Mapeamos el valor de 0-15 a 0-255
    int lcdPWM = map(intensity, 0, 15, 0, 255);

    // Corrección: Los LCDs se apagan completamente si el voltaje es muy bajo.
    // Si la intensidad es > 0, aseguramos un mínimo de luz para que se lea.
    if (intensity > 2 && lcdPWM < 15) {
        lcdPWM = 15; 
    }
    
    // Escribir el valor PWM en el pin
    analogWrite(LCD_PWM_PIN, lcdPWM);    
}

void DisplayMgr::setLcdBacklight(bool on) {
    if (on) {
        analogWrite(LCD_PWM_PIN, 200);
    } else {
        analogWrite(LCD_PWM_PIN, 50);
    }
}

void DisplayMgr::printMatrix(String text) {
    // Convertimos String a char array para Parola
    matrix.setTextAlignment(PA_CENTER);
    matrix.print(text.c_str());
}

void DisplayMgr::updateRoutine() {
    matrix.displayAnimate();
}