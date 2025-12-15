#ifndef MENUMGR_H
#define MENUMGR_H

#include <Arduino.h>
#include "MenuDefines.h"
#include "DisplayMgr.h"
#include "InputMgr.h"

class MenuMgr {
public:
    static void init();
    static void handleNavigation(ButtonState btn);
    static void executeAction();
    static void draw();

private:
    static MenuItem menuItems[]; 
    
    static int currentIndex;     
    static int topVisibleIndex;  
    
    // static const int indica que es un número fijo compartido por todo el sistema
    static const int itemsCount; 
};

#endif