#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LIBRERÍAS DE TIEMPO Y CONTEO
#include "time.h"
#include "esp_sntp.h" 

// --- 0. HARDWARE DEFINITIONS ---
#define MODE_BUTTON_PIN 13 // CAMBIO DE HORA MANUAL (si WIFI está OFF) / SALIDA
#define UP_BUTTON_PIN 12   // SUBIR/INCREMENTAR/NAVEGAR
#define DOWN_BUTTON_PIN 14 // BAJAR/DECREMENTAR/NAVEGAR / ENTRAR a CONFIG. GENERAL (desde RELOJ)
#define SET_BUTTON_PIN 27  // ENTER/CONFIRMAR / SILENCIAR
#define BUZZER_PIN_SIGNAL 26 

// --- 1. CONFIGURACIÓN LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- 2. CONFIGURACIÓN WIFI CLIENTE (NTP) ---
const char* ssid = "Aramel's S 20 FE";    
const char* password = "No te lo puedo dar 02";
// --- 3. MÁQUINA DE ESTADOS Y VARIABLES ---
enum Modos {
  RELOJ,            
  ALARM_CONFIG,     // Configurar Hora/Minuto de Alarma
  GENERAL_CONFIG,   // Nuevo: Formato de Hora / Configuración de Wi-Fi / Cambio Manual
  ALARMA_SONANDO    
};

enum AlarmaSubEstados {
    ALM_SET_HOUR,     
    ALM_SET_MINUTE    
};

enum GeneralSubEstados {
    GEN_MENU,          // Menú principal de Configuración General
    GEN_SET_FORMAT,    // Ajuste de Formato (12h/24h)
    GEN_SET_WIFI,      // Encender/Apagar Wi-Fi (NTP)
    GEN_SET_MANUAL_TIME// Ajuste manual de HORA y MINUTO (Solo si Wi-Fi está OFF)
};

// Variables Globales
Modos modoActual = RELOJ;
AlarmaSubEstados alarmaSubEstado = ALM_SET_HOUR; 
GeneralSubEstados generalSubEstado = GEN_MENU;

int alarmaHora = 7;
int alarmaMinuto = 30;
bool alarmaActiva = true; 
bool alarmaCheckeada = false; 

// ************ NUEVAS VARIABLES ************
bool wifiActivo = true; // Controla si intentamos usar NTP
bool formato24H = true; // true: 24h (HH:MM), false: 12h (HH:MM AM/PM)
int menuCursor = 0;     // 0: Formato, 1: Wi-Fi, 2: Ajuste Manual (solo si Wi-Fi OFF)

// Variables de Tiempo
const char* ntpServer = "pool.ntp.org";
long timeOffset = -18000; // UTC-5 por defecto
int daylightOffset = 0;
unsigned long lastButtonPress = 0; 
int currentManualHour = 10; // Usado para el ajuste manual
int currentManualMinute = 0;


// --- 4. MELODÍA Y AUDIO ---
// ... (Melodía de Star Wars y funciones de playTone/playStarWarsAlarm son las mismas) ...
#define NOTE_B4  494
#define NOTE_A4  440
#define NOTE_G4  392
#define NOTE_FSH4 370
#define NOTE_D5  587
#define NOTE_C5  523
int melody[] = {
  NOTE_A4, NOTE_A4, NOTE_A4, NOTE_FSH4, NOTE_C5,
  NOTE_A4, NOTE_FSH4, NOTE_C5, NOTE_A4
};
int durations[] = {4, 4, 4, 5, 15, 4, 5, 15, 8};
int tempo = 120; 

void playTone(int freq, long duration) {
    tone(BUZZER_PIN_SIGNAL, freq);
    delay(duration);
    noTone(BUZZER_PIN_SIGNAL);
    delay(duration / 8); 
}

void playStarWarsAlarm() {
    int notes = sizeof(melody) / sizeof(melody[0]);
    for (int i = 0; i < notes; i++) {
        if (modoActual != ALARMA_SONANDO) {
            noTone(BUZZER_PIN_SIGNAL);
            break; 
        }
        int noteDuration = 60000 / tempo / durations[i];
        playTone(melody[i], noteDuration);
    }
    if (modoActual == ALARMA_SONANDO) {
        delay(500); 
    }
}

void sonarAlarma() {
    if (modoActual == ALARMA_SONANDO) {
        playStarWarsAlarm(); 
    } else {
        noTone(BUZZER_PIN_SIGNAL);
    }
}


// ===============================================
// === 5. FUNCIONES DE TIEMPO Y CONFIGURACIÓN ===
// ===============================================

// Función clave: Usa NTP si wifiActivo es TRUE. Si es FALSE, usa la hora manual.
String obtenerHoraActual(bool conSegundos) {
    char buffer[17]; // Suficiente para HH:MM:SS AM
    struct tm timeinfo;
    
    if (wifiActivo) {
        if(!getLocalTime(&timeinfo)){
            return conSegundos ? "--:--:--" : "--:--"; 
        }
    } else {
        // Usar la hora manual interna (simplificada: sin segundos, sin RTC)
        timeinfo.tm_hour = currentManualHour;
        timeinfo.tm_min = currentManualMinute;
    }
    
    const char* fmt;
    if (formato24H) {
        fmt = conSegundos ? "%H:%M:%S" : "%H:%M";
    } else {
        fmt = conSegundos ? "%I:%M:%S %p" : "%I:%M %p"; // %I: 12h, %p: AM/PM
    }
    
    strftime(buffer, sizeof(buffer), fmt, &timeinfo);
    return String(buffer);
}

// Actualiza la hora manual interna (llamada en el loop si wifiActivo es FALSE)
void actualizarHoraManual() {
    static unsigned long lastMinute = 0;
    if (millis() - lastMinute >= 60000) { // Incrementa cada 60 segundos
        currentManualMinute++;
        if (currentManualMinute >= 60) {
            currentManualMinute = 0;
            currentManualHour++;
            if (currentManualHour >= 24) {
                currentManualHour = 0;
            }
        }
        lastMinute = millis();
        Serial.printf("Hora Manual: %02d:%02d\n", currentManualHour, currentManualMinute);
        actualizarLCD();
    }
}

// Inicialización de NTP
void configTimeNTP() {
    configTime(timeOffset, daylightOffset, ntpServer);
}

void actualizarNTP(struct timeval *t) {
    Serial.println("Hora NTP sincronizada con éxito.");
}

void checkAlarma() {
    if (!alarmaActiva || modoActual != RELOJ) return;

    // Obtener la hora de la alarma de forma consistente
    int horaCheck;
    int minutoCheck;

    if (wifiActivo) {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) return;
        horaCheck = timeinfo.tm_hour;
        minutoCheck = timeinfo.tm_min;
    } else {
        horaCheck = currentManualHour;
        minutoCheck = currentManualMinute;
    }

    if (horaCheck == alarmaHora && minutoCheck == alarmaMinuto && !alarmaCheckeada) {
        modoActual = ALARMA_SONANDO;
        alarmaCheckeada = true;
        Serial.println("!!! ALARMA DISPARADA !!!");
    } else if (minutoCheck != alarmaMinuto) {
        alarmaCheckeada = false;
    }
}


// ===============================================
// === 7. MANEJO DE BOTONES (FSM extendida) ===
// ===============================================

void manejarBotonesHardware() {
    const unsigned long debounceDelay = 200;
    
    bool mode = digitalRead(MODE_BUTTON_PIN) == LOW; // HORA_MANUAL / SALIR
    bool up = digitalRead(UP_BUTTON_PIN) == LOW;     
    bool down = digitalRead(DOWN_BUTTON_PIN) == LOW; // CONFIG. GENERAL / BAJAR
    bool set = digitalRead(SET_BUTTON_PIN) == LOW;   
    
    if (!mode && !up && !down && !set) {
        lastButtonPress = 0;
        return;
    }
    if (millis() - lastButtonPress < debounceDelay) return;
    lastButtonPress = millis();

    Serial.printf("MODO: %d, ESTADO: %d | MODE:%d UP:%d DOWN:%d SET:%d\n", modoActual, alarmaSubEstado, mode, up, down, set);

    // 1. Manejar Alarma Sonando (HUSH)
    if (modoActual == ALARMA_SONANDO) {
        modoActual = RELOJ;
        noTone(BUZZER_PIN_SIGNAL);
        return;
    }
    
    // 2. Transiciones de Modos Principales (desde RELOJ)
    if (modoActual == RELOJ) {
        if (mode) { // MODE: Entra a Configuración de ALARMA
            modoActual = ALARM_CONFIG;
            alarmaSubEstado = ALM_SET_HOUR;
            Serial.println("RELOJ -> ALARM_CONFIG");
        } else if (down) { // DOWN: Entra a Configuración GENERAL
            modoActual = GENERAL_CONFIG;
            generalSubEstado = GEN_MENU;
            menuCursor = 0;
            Serial.println("RELOJ -> GENERAL_CONFIG");
        }
    } 
    // Salida de Configuración (MODE)
    else if (mode && (modoActual == ALARM_CONFIG || modoActual == GENERAL_CONFIG)) {
        modoActual = RELOJ;
        Serial.println("CONFIG -> RELOJ (Salida MODE)");
        actualizarLCD();
        return;
    }
    
    // 3. FSM de Configuración de Alarma (3 Pasos: H -> M -> RELOJ)
    if (modoActual == ALARM_CONFIG) {
        // ... (Lógica de ALARMA_CONFIG es la misma) ...
        int cambio = (up) ? 1 : ((down) ? -1 : 0);
        if (cambio != 0) {
            if (alarmaSubEstado == ALM_SET_HOUR) {
                alarmaHora = (alarmaHora + cambio + 24) % 24;
            } else if (alarmaSubEstado == ALM_SET_MINUTE) {
                alarmaMinuto = (alarmaMinuto + cambio + 60) % 60;
            }
        }
        
        if (set) {
            if (alarmaSubEstado == ALM_SET_HOUR) {
                alarmaSubEstado = ALM_SET_MINUTE;
            } else if (alarmaSubEstado == ALM_SET_MINUTE) {
                modoActual = RELOJ; // Vuelve a RELOJ
            }
        }
    }

    // ************ NUEVA FSM: Configuración General ************
    else if (modoActual == GENERAL_CONFIG) {
        
        if (generalSubEstado == GEN_MENU) {
            int maxOptions = wifiActivo ? 2 : 3; // 2 opciones si hay Wi-Fi, 3 si no lo hay
            
            // NAVEGACIÓN DEL MENÚ (UP/DOWN)
            if (up) menuCursor = (menuCursor + maxOptions - 1) % maxOptions;
            if (down) menuCursor = (menuCursor + 1) % maxOptions;

            // ENTRAR A SUB-ESTADO (SET)
            if (set) {
                if (menuCursor == 0) generalSubEstado = GEN_SET_FORMAT;
                else if (menuCursor == 1) generalSubEstado = GEN_SET_WIFI;
                else if (menuCursor == 2 && !wifiActivo) generalSubEstado = GEN_SET_MANUAL_TIME;
            }
        }
        
        // AJUSTE DE FORMATO (SET_FORMAT)
        else if (generalSubEstado == GEN_SET_FORMAT) {
            if (up || down) {
                formato24H = !formato24H; // Alternar 24H y AM/PM
            }
            if (set) {
                generalSubEstado = GEN_MENU; // Volver al menú
            }
        }

        // AJUSTE DE WI-FI (SET_WIFI)
        else if (generalSubEstado == GEN_SET_WIFI) {
            if (up || down) {
                wifiActivo = !wifiActivo;
                if (!wifiActivo) {
                    WiFi.disconnect(true);
                    Serial.println("Wi-Fi Desactivado (Usando Hora Manual)");
                } else {
                    // Si se reactiva, volvemos a intentar la conexión NTP en el loop
                    Serial.println("Wi-Fi Activado (Intentando NTP)");
                }
            }
            if (set) {
                generalSubEstado = GEN_MENU;
            }
        }
        
        // AJUSTE MANUAL DE HORA (SET_MANUAL_TIME)
        else if (generalSubEstado == GEN_SET_MANUAL_TIME) {
            // Esta lógica se reutiliza de ALARM_CONFIG
            int cambio = (up) ? 1 : ((down) ? -1 : 0);
            
            if (cambio != 0) {
                // Si estamos en la hora, ajustamos la hora global manual
                if (menuCursor == 0) { // Usamos menuCursor aquí para alternar H/M
                    currentManualHour = (currentManualHour + cambio + 24) % 24;
                } else {
                    currentManualMinute = (currentManualMinute + cambio + 60) % 60;
                }
            }
            
            if (set) {
                if (menuCursor == 0) {
                    menuCursor = 1; // Pasar a Minuto
                } else {
                    menuCursor = 0; // Resetear y volver al menú
                    generalSubEstado = GEN_MENU; 
                }
            }
        }
    }
    // *************************************************************

    actualizarLCD();
}


// ===============================================
// === 8. DISPLAY LCD ===
// ===============================================

void actualizarLCD() {
    bool parpadeando = (millis() / 500) % 2; 

    lcd.clear();
    lcd.setCursor(0, 0);
    
    // MODO RELOJ
    if (modoActual == RELOJ) {
        String horaStr = obtenerHoraActual(true);
        lcd.print("RELOJ " + horaStr);
        lcd.setCursor(0, 1);
        lcd.printf("ALM %02d:%02d V:MAX %s", alarmaHora, alarmaMinuto, alarmaActiva ? "ON" : "OFF");
        if (!wifiActivo) lcd.print(" NO-WIFI"); // Indicador de Wi-Fi desactivado
    } 
    
    // MODO CONFIGURACIÓN DE ALARMA (3 PASOS)
    else if (modoActual == ALARM_CONFIG) { 
        lcd.print("ALM CONFIG (3-STEP)");
        lcd.setCursor(0, 1);
        // ... (Lógica de visualización de ALARM_CONFIG) ...
        if (parpadeando) {
            if (alarmaSubEstado == ALM_SET_HOUR) {
                lcd.printf("SET: --:%02d [H]", alarmaMinuto); 
            } else { 
                lcd.printf("SET: %02d:-- [M]", alarmaHora); 
            }
        } else {
            lcd.printf("SET: %02d:%02d", alarmaHora, alarmaMinuto);
        }
    } 
    
    // ************ MODO CONFIGURACIÓN GENERAL ************
    else if (modoActual == GENERAL_CONFIG) {
        
        lcd.print("CONFIG GENERAL");

        lcd.setCursor(0, 1);
        int maxOptions = wifiActivo ? 2 : 3;

        if (generalSubEstado == GEN_MENU) {
            if (menuCursor == 0) lcd.printf("> FORMATO: %s", formato24H ? "24H" : "AM/PM");
            else if (menuCursor == 1) lcd.printf("> WI-FI: %s", wifiActivo ? "ON (NTP)" : "OFF");
            else if (menuCursor == 2 && !wifiActivo) lcd.print("> AJUSTE HORA MANUAL");
            else lcd.print("ERROR MENU");
        }
        
        else if (generalSubEstado == GEN_SET_FORMAT) {
            String fStr = formato24H ? "24 HORAS" : "AM/PM";
            if (parpadeando) fStr = "        ";
            lcd.print("FORMATO: " + fStr);
        }
        
        else if (generalSubEstado == GEN_SET_WIFI) {
            String wStr = wifiActivo ? "ACTIVO" : "INACTIVO";
            if (parpadeando) wStr = "        ";
            lcd.print("WI-FI: " + wStr);
        }
        
        else if (generalSubEstado == GEN_SET_MANUAL_TIME) {
            // Mostrar la hora manual con la parte actual parpadeando
            if (parpadeando) {
                if (menuCursor == 0) { // Hora
                    lcd.printf("HORA: --:%02d [H]", currentManualMinute); 
                } else { // Minuto
                    lcd.printf("HORA: %02d:-- [M]", currentManualHour); 
                }
            } else {
                lcd.printf("HORA: %02d:%02d", currentManualHour, currentManualMinute);
            }
        }
    } 
    // *************************************************************
    
    // MODO ALARMA SONANDO
    else if (modoActual == ALARMA_SONANDO) {
        lcd.print("!!! STAR WARS !!!");
        lcd.setCursor(0, 1);
        lcd.print(obtenerHoraActual(false) + " HUSH [SET]");
    }
}


// ===============================================
// === 9. SETUP Y LOOP ===
// ===============================================

// ... (El setup es casi idéntico, solo cambia la última línea) ...

void setup() {
    Serial.begin(115200);
    Wire.begin(); 

    // Inicializar LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Iniciando...");

    // Inicializar Pines de Hardware (Botones y Buzzer)
    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SET_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN_SIGNAL, OUTPUT);
    digitalWrite(BUZZER_PIN_SIGNAL, LOW); 

    // LÓGICA DE CONEXIÓN WI-FI
    lcd.setCursor(0, 1);
    lcd.print("Conectando...");
    Serial.print("Conectando a Wi-Fi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n¡Wi-Fi conectado! Sincronizando hora.");
        lcd.clear();
        lcd.print("Conectado! NTP OK");
        
        sntp_set_time_sync_notification_cb(actualizarNTP);
        configTimeNTP();
        wifiActivo = true;

    } else {
        Serial.println("\nError al conectar al Wi-Fi. Usando hora manual.");
        lcd.clear();
        lcd.print("ERROR WI-FI");
        lcd.setCursor(0, 1);
        lcd.print("Usando Hora Manual");
        wifiActivo = false;
    }

    delay(3000); 
    modoActual = RELOJ;
    actualizarLCD();
}

void loop() {
    manejarBotonesHardware();

    // Actualiza la hora manual si el Wi-Fi está desactivado
    if (modoActual == RELOJ && !wifiActivo) {
        actualizarHoraManual();
    }
    
    // 2. Lógica de Modos (RELOJ)
    if (modoActual == RELOJ) {
        static time_t last_time = 0;
        time_t now = time(nullptr);
        
        // Si usamos NTP (Wi-Fi), actualizamos cada segundo.
        if (wifiActivo && now != last_time) {
            checkAlarma();
            actualizarLCD();
            last_time = now;
        } 
        // Si es hora manual, 'actualizarHoraManual' maneja el checkeo y la actualización
        else if (!wifiActivo) {
            checkAlarma(); 
        }
    }

    // ... (El resto del loop es el mismo) ...
    else if (modoActual == ALARMA_SONANDO) {
        sonarAlarma(); 
        if ((millis() / 500) % 2) {
            actualizarLCD();
        }
    } 
    
    else if (modoActual == ALARM_CONFIG || modoActual == GENERAL_CONFIG) {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate >= 500) {
             actualizarLCD();
             lastUpdate = millis();
        }
    }
    
    delay(5);
}