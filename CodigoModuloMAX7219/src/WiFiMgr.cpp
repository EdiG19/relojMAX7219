#include "WiFiMgr.h"
#include "DisplayMgr.h" // Para mostrar mensajes de estado en LCD

void WiFiMgr::init() {
    // Modo Estación (Cliente)
    WiFi.mode(WIFI_STA);
    
    // Iniciamos la configuración de tiempo (aunque no tengamos internet aún)
    setupTime();

    // Intentamos conectar con lo que haya guardado
    // No usamos wm.autoConnect() aquí para no bloquear el arranque si no hay red.
    WiFi.begin(); 
}

void WiFiMgr::startConfigPortal() {
    WiFiManager wm;
    
    // Feedback visual en pantallas
    DisplayMgr::clearLCD();
    DisplayMgr::showMenuState("MODO AP WIFI", "Busca: RelojAP");
    DisplayMgr::printMatrix("CONF");

    // Configuramos timeout para que no se quede pegado por siempre (ej. 3 minutos)
    wm.setConfigPortalTimeout(180); 

    // Crea el punto de acceso
    if (!wm.startConfigPortal("RelojAP")) {
        DisplayMgr::printLCD(1, "Timeout/Error");
        delay(3000);
        // Si falla, reiniciamos o volvemos
        return; 
    }

    // Si llegamos aquí, se conectó exitosamente
    DisplayMgr::printLCD(1, "Conectado!");
    DisplayMgr::printMatrix("WIFI OK");
    delay(2000);
}

bool WiFiMgr::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiMgr::setupTime() {
    // Configura el cliente NTP integrado del ESP32
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET, NTP_SERVER);
}

String WiFiMgr::getLocalTimeStr() {
    struct tm timeinfo;
    
    // getLocalTime devuelve true si ya se sincronizó la hora
    // El '500' es el tiempo de espera en ms
    if(!getLocalTime(&timeinfo, 10)) {
        return "--:--"; // Aún no hay hora
    }
    char timeStringBuffer[9];
    // Formato HH:MM:SS
    strftime(timeStringBuffer, sizeof(timeStringBuffer), "%H:%M:%S", &timeinfo);
    
    return String(timeStringBuffer);
}