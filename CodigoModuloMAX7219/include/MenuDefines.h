#ifndef MENUDEFINES_H
#define MENUDEFINES_H

#include <Arduino.h>

// 1. Definimos el "Tipo" de función Callback
// Significa: "Una función que no devuelve nada (void) y no recibe parámetros ()"
typedef void (*MenuAction)(); 

// 2. Definimos la estructura de un Ítem del menú
struct MenuItem {
    String title;       // Texto a mostrar (ej: "1. Cronometro")
    MenuAction action;  // La función a ejecutar al dar OK
};

#endif