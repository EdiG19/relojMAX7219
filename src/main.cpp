#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LIBRERÍAS DE TIEMPO Y CONTEO
#include "time.h"
#include "esp_sntp.h" 

// --- 0. HARDWARE DEFINITIONS ---
#define MODE_BUTTON_PIN 13 // CONFIG. ALARMA / SALIDA (RESET)
#define UP_BUTTON_PIN 12   // SUBIR/INCREMENTAR
#define DOWN_BUTTON_PIN 14 // BAJAR/DECREMENTAR / ENTRAR a CONFIG. GENERAL (desde RELOJ)
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
  ALARM_CONFIG,     
  GENERAL_CONFIG,   // CONFIGURACIÓN CENTRAL (Flujo Invertido)
  ALARMA_SONANDO    
};

enum AlarmaSubEstados {
    ALM_SET_HOUR,     
    ALM_SET_MINUTE    
};

// ** NUEVOS SUB-ESTADOS PARA EL FLUJO INVERTIDO **
enum GeneralSubEstados {
    GEN_SET_MANUAL_H,  // Nuevo Paso 1: Ajuste manual de HORA
    GEN_SET_MANUAL_M,  // Nuevo Paso 2: Ajuste manual de MINUTO
    GEN_SET_FORMAT,    // Nuevo Paso 3: Ajuste de Formato (12h/24h)
    GEN_SET_WIFI,      // Nuevo Paso 4: Pregunta: ¿Wi-Fi ON/OFF?
    GEN_FINISHED       // Finalizado (Vuelve a RELOJ)
};

// Variables Globales
Modos modoActual = RELOJ;
AlarmaSubEstados alarmaSubEstado = ALM_SET_HOUR; 
GeneralSubEstados generalSubEstado = GEN_SET_MANUAL_H; // ** INICIA EN HORA MANUAL **

int alarmaHora = 7;
int alarmaMinuto = 30;
bool alarmaActiva = true; 
bool alarmaCheckeada = false; 

// Variables de Configuración de Hora
bool wifiActivo = true; 
bool formato24H = true; 
int menuCursor = 0;     

// Variables de Tiempo
const char* ntpServer = "pool.ntp.org";
long timeOffset = -18000; 
int daylightOffset = 0;
unsigned long lastButtonPress = 0; 
int currentManualHour = 10; 
int currentManualMinute = 0;


// --- 4. MELODÍA Y AUDIO (Sin Cambios) ---
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

void actualizarLCD(); // Prototipo

void actualizarHoraManual() {
    static unsigned long lastMinute = 0;
    if (millis() - lastMinute >= 60000) { 
        currentManualMinute++;
        if (currentManualMinute >= 60) {
            currentManualMinute = 0;
            currentManualHour++;
            if (currentManualHour >= 24) {
                currentManualHour = 0;
            }
        }
        lastMinute = millis();
        actualizarLCD(); 
    }
}

String obtenerHoraActual(bool conSegundos) {
    char buffer[17]; 
    struct tm timeinfo;
    
    // Si Wi-Fi está activo, se usa NTP (real time)
    if (wifiActivo) {
        if(!getLocalTime(&timeinfo)){
            return conSegundos ? "--:--:--" : "--:--"; 
        }
    } else {
        // Si Wi-Fi está inactivo, se usa la hora manual (sólo HORA y MINUTO)
        timeinfo.tm_hour = currentManualHour;
        timeinfo.tm_min = currentManualMinute;
    }
    
    const char* fmt;
    if (formato24H) {
        fmt = conSegundos ? "%H:%M:%S" : "%H:%M";
    } else {
        fmt = conSegundos ? "%I:%M:%S %p" : "%I:%M %p"; 
    }
    
    // Si estamos en modo manual, forzamos los segundos a cero o quitamos los AM/PM si no aplican al formato.
    if (!wifiActivo && conSegundos) {
        if (formato24H) {
             sprintf(buffer, "%02d:%02d:00", timeinfo.tm_hour, timeinfo.tm_min);
        } else {
             int hora12 = (timeinfo.tm_hour % 12 == 0) ? 12 : timeinfo.tm_hour % 12;
             const char* ampm = (timeinfo.tm_hour < 12) ? "AM" : "PM";
             sprintf(buffer, "%02d:%02d:00 %s", hora12, timeinfo.tm_min, ampm);
        }
        return String(buffer);
    }

    strftime(buffer, sizeof(buffer), fmt, &timeinfo);
    return String(buffer);
}

void configTimeNTP() {
    configTime(timeOffset, daylightOffset, ntpServer);
}

void actualizarNTP(struct timeval *t) {
    Serial.println("Hora NTP sincronizada con éxito.");
}

void checkAlarma() {
    if (!alarmaActiva || modoActual != RELOJ) return;

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
    
    bool mode = digitalRead(MODE_BUTTON_PIN) == LOW; 
    bool up = digitalRead(UP_BUTTON_PIN) == LOW;     
    bool down = digitalRead(DOWN_BUTTON_PIN) == LOW; 
    bool set = digitalRead(SET_BUTTON_PIN) == LOW;   
    
    if (!mode && !up && !down && !set) {
        lastButtonPress = 0;
        return;
    }
    if (millis() - lastButtonPress < debounceDelay) return;
    lastButtonPress = millis();

    Serial.printf("MODO: %d, ESTADO: %d | MODE:%d UP:%d DOWN:%d SET:%d\n", modoActual, generalSubEstado, mode, up, down, set);

    // 1. Manejar Alarma Sonando (HUSH)
    if (modoActual == ALARMA_SONANDO) {
        modoActual = RELOJ;
        noTone(BUZZER_PIN_SIGNAL);
        actualizarLCD(); 
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
            generalSubEstado = GEN_SET_MANUAL_H; // ** INICIA: Ajuste de Hora **
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
    
    // 3. FSM de Configuración de Alarma (sin cambios)
    if (modoActual == ALARM_CONFIG) {
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
                modoActual = RELOJ; 
            }
        }
    }

    // ************ FSM: Configuración General (FLUJO REQUERIDO) ************
    else if (modoActual == GENERAL_CONFIG) {
        
        int cambio = (up) ? 1 : ((down) ? -1 : 0);
        
        // GEN_SET_MANUAL_H (Paso 1: Ajuste HORA)
        if (generalSubEstado == GEN_SET_MANUAL_H) {
            if (cambio != 0) {
                currentManualHour = (currentManualHour + cambio + 24) % 24;
            }
            if (set) {
                generalSubEstado = GEN_SET_MANUAL_M; // Pasa a MINUTO
            }
        }

        // GEN_SET_MANUAL_M (Paso 2: Ajuste MINUTO)
        else if (generalSubEstado == GEN_SET_MANUAL_M) {
            if (cambio != 0) {
                currentManualMinute = (currentManualMinute + cambio + 60) % 60;
            }
            if (set) {
                generalSubEstado = GEN_SET_FORMAT; // Pasa a FORMATO
            }
        }

        // GEN_SET_FORMAT (Paso 3: Ajuste Formato)
        else if (generalSubEstado == GEN_SET_FORMAT) {
            if (up || down) {
                formato24H = !formato24H; 
            }
            if (set) {
                generalSubEstado = GEN_SET_WIFI; // Pasa a Wi-Fi
            }
        }
        
        // GEN_SET_WIFI (Paso 4: Pregunta ¿Wi-Fi ON/OFF?)
        else if (generalSubEstado == GEN_SET_WIFI) {
            if (up || down) {
                wifiActivo = !wifiActivo; // Alternar SÍ (ON) / NO (OFF)
                if (!wifiActivo) {
                    WiFi.disconnect(true);
                    Serial.println("Wi-Fi Desactivado (Usando Hora Manual)");
                } else {
                    Serial.println("Wi-Fi Activado (Intentando NTP)");
                }
            }
            
            if (set) {
                if (wifiActivo) {
                    // Elegir SÍ (Wi-Fi ON) -> Finaliza la configuración
                    generalSubEstado = GEN_FINISHED; 
                    modoActual = RELOJ;
                } else {
                    // Elegir NO (Wi-Fi OFF) -> Finaliza el flujo de configuración y vuelve a RELOJ
                    // (Ya hemos ajustado la hora manual al inicio del flujo)
                    generalSubEstado = GEN_FINISHED; 
                    modoActual = RELOJ;
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
        if (!wifiActivo) lcd.print(" NO-WIFI"); 
    } 
    
    // MODO CONFIGURACIÓN DE ALARMA
    else if (modoActual == ALARM_CONFIG) { 
        lcd.print("ALM CONFIG (3-STEP)");
        lcd.setCursor(0, 1);
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
    
    // ************ MODO CONFIGURACIÓN GENERAL (FLUJO REQUERIDO) ************
    else if (modoActual == GENERAL_CONFIG) {
        
        lcd.print("CONFIG HORA");
        lcd.setCursor(0, 1);

        // GEN_SET_MANUAL_H (Paso 1: HORA)
        if (generalSubEstado == GEN_SET_MANUAL_H) {
            lcd.print("1. HORA MANUAL [H]");
            lcd.setCursor(0, 1);
            if (parpadeando) {
                lcd.printf("   --:%02d", currentManualMinute); 
            } else {
                lcd.printf("   %02d:%02d", currentManualHour, currentManualMinute);
            }
        }
        
        // GEN_SET_MANUAL_M (Paso 2: MINUTO)
        else if (generalSubEstado == GEN_SET_MANUAL_M) {
            lcd.print("2. MINUTO MANUAL [M]");
            lcd.setCursor(0, 1);
            if (parpadeando) {
                lcd.printf("   %02d:--", currentManualHour); 
            } else {
                lcd.printf("   %02d:%02d", currentManualHour, currentManualMinute);
            }
        }
        
        // GEN_SET_FORMAT (Paso 3: Formato)
        else if (generalSubEstado == GEN_SET_FORMAT) {
            lcd.print("3. FORMATO: ");
            String fStr = formato24H ? "24 HORAS" : "AM/PM";
            if (parpadeando) fStr = "        ";
            lcd.print(fStr);
        }
        
        // GEN_SET_WIFI (Paso 4: Pregunta Wi-Fi)
        else if (generalSubEstado == GEN_SET_WIFI) {
            lcd.print("4. HORA AUTOMATICA?");
            lcd.setCursor(0, 1);
            lcd.print("    [UP/DOWN]: ");
            String wStr = wifiActivo ? "SI (ON)" : "NO (OFF)";
            if (parpadeando) wStr = "         ";
            lcd.print(wStr);
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

void setup() {
    Serial.begin(115200);
    Wire.begin(); 

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Iniciando...");

    pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
    pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
    pinMode(SET_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN_SIGNAL, OUTPUT);
    digitalWrite(BUZZER_PIN_SIGNAL, LOW); 

    // LÓGICA DE CONEXIÓN WI-FI
    lcd.setCursor(0, 1);
    lcd.print("Conectando...");
    
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        lcd.clear();
        lcd.print("Conectado! NTP OK");
        sntp_set_time_sync_notification_cb(actualizarNTP);
        configTimeNTP();
        wifiActivo = true;

    } else {
        lcd.clear();
        lcd.print("ERROR WI-FI");
        lcd.setCursor(0, 1);
        lcd.print("Usando Hora Manual");
        wifiActivo = false;
        // La hora manual ya está preestablecida
        currentManualHour = 10; 
        currentManualMinute = 0;
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
        
        if (wifiActivo) {
            if (now != last_time) {
                checkAlarma();
                actualizarLCD();
                last_time = now;
            }
        } else {
            // En modo manual, solo chequeamos la alarma (actualizarHoraManual() ya actualiza el LCD)
            checkAlarma(); 
        }
    }

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