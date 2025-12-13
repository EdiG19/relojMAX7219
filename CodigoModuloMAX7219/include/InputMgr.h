#ifndef INPUTMGR_H
#define INPUTMGR_H

#include <Arduino.h>

// --- DEFINICIÓN DE PINES (ajústalos a tu conexión real) ---
#define PIN_BTN_UP     19  // D13
#define PIN_BTN_DOWN   17  // D12
#define PIN_BTN_OK     16  // D14
#define PIN_BTN_BACK   4  // D27 (Opcional)

#define PIN_LDR        32  // Pin Analógico (ADC1) para el sensor de luz

// --- ESTADOS DE LOS BOTONES ---
enum ButtonState {
    BTN_NONE = 0,
    BTN_UP,
    BTN_DOWN,
    BTN_OK,
    BTN_BACK
};

class InputMgr {
public:
    static void init();

    // Esta función la llamaremos en el loop.
    // Devuelve el botón presionado SOLO una vez (con debounce).
    static ButtonState readButtons();

    // Devuelve un valor de brillo (0-15) basado en la luz ambiente
    static int getAutoBrightness();

    static int readLDR();//funcion auxiliar para corregir los rangos del LDR


private:
    // Variables para el debounce (evitar lecturas falsas)
    static unsigned long lastDebounceTime;
    static ButtonState lastState;
};

#endif