#include "InputMgr.h"

// Inicialización de variables estáticas
unsigned long InputMgr::lastDebounceTime = 0;
ButtonState InputMgr::lastState = BTN_NONE;
const unsigned long DEBOUNCE_DELAY = 150; // 150ms de espera entre pulsaciones

void InputMgr::init() {
    // Configuramos pines como entradas con resistencia PULLUP interna
    // (Conecta el botón entre el PIN y GND)
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DOWN, INPUT_PULLUP);
    pinMode(PIN_BTN_OK, INPUT_PULLUP);
    pinMode(PIN_BTN_BACK, INPUT_PULLUP);
    
    // El LDR es solo entrada analógica, no necesita pinMode en ESP32, 
    // pero algunos cores lo agradecen.
    pinMode(PIN_LDR, INPUT);
}

ButtonState InputMgr::readButtons() {
    // Si ha pasado poco tiempo desde la última pulsación, ignoramos todo
    if (millis() - lastDebounceTime < DEBOUNCE_DELAY) {
        return BTN_NONE;
    }

    ButtonState pressed = BTN_NONE;

    // Leemos los pines (LOW significa presionado por el INPUT_PULLUP)
    if (digitalRead(PIN_BTN_UP) == LOW)      pressed = BTN_UP;
    else if (digitalRead(PIN_BTN_DOWN) == LOW) pressed = BTN_DOWN;
    else if (digitalRead(PIN_BTN_OK) == LOW)   pressed = BTN_OK;
    else if (digitalRead(PIN_BTN_BACK) == LOW) pressed = BTN_BACK;

    // Si detectamos algo
    if (pressed != BTN_NONE) {
        lastDebounceTime = millis(); // Reiniciamos el cronómetro de debounce
        return pressed;
    }

    return BTN_NONE;
}
int InputMgr::readLDR() {
    int val = analogRead(PIN_LDR);
    if (val < 30) val =30;
    if (val > 1200) val = 1200;
    return val;
}
int InputMgr::getAutoBrightness() {
    // Leemos el valor analógico (0 - 4095 en ESP32)
    int val = readLDR();
    
    // Mapeamos el valor a brillo de matriz (0 a 15)
    float valorNormalizado = (float)(val-30)/(1200-30);
    float curvedVal= pow(valorNormalizado,0.5);
    int brightness = (int)(curvedVal * 15);
    return brightness;
}
