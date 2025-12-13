#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LIBRERÍAS DE TIEMPO Y CONTEO
#include "time.h"
#include "esp_sntp.h" 

// --- 0. HARDWARE DEFINITIONS ---
#define MODE_BUTTON_PIN 13 
#define UP_BUTTON_PIN 12   
#define DOWN_BUTTON_PIN 14 
#define SET_BUTTON_PIN 27  
#define BUZZER_PIN_SIGNAL 26 

// --- 1. CONFIGURACIÓN LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- 2. CONFIGURACIÓN WIFI CLIENTE (NTP) ---
// *** ¡CAMBIA ESTOS VALORES por tu red Wi-Fi con internet! ***
const char* ssid = "Aramel's S 20 FE";    
const char* password = "No te lo puedo dar 02";

// --- 3. MÁQUINA DE ESTADOS Y VARIABLES (Resto de la lógica es la misma) ---
enum Modos {
  RELOJ,          
  ALARMA_CONFIG,  
  ALARMA_SONANDO  
};

enum AlarmaSubEstados {
    ALM_SET_HOUR,     
    ALM_SET_MINUTE    
};

Modos modoActual = RELOJ; // Empezamos en RELOJ para ver la hora sincronizada
AlarmaSubEstados alarmaSubEstado = ALM_SET_HOUR; 

// Variables de Configuración
int alarmaHora = 7;
int alarmaMinuto = 30;
bool alarmaActiva = true; 
bool alarmaCheckeada = false; 

// Variables de Tiempo
const char* ntpServer = "pool.ntp.org";
long timeOffset = -18000; // UTC-5 por defecto (Panamá, EST)
int daylightOffset = 0;

unsigned long lastButtonPress = 0; 

// --- 4. MELODÍA DE ALARMA: MARCHA IMPERIAL ---
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
int tempo = 40; 


// --- 5. PROTOTIPOS DE FUNCIONES ---
void configTimeNTP();
void actualizarNTP(struct timeval *t);
void checkAlarma();
void playTone(int freq, long duration);
void playStarWarsAlarm();
void sonarAlarma();
void actualizarLCD();
void manejarBotonesHardware();
String obtenerHoraActual(bool conSegundos = false);


// ===============================================
// === 6. FUNCIONES DE TIEMPO Y AUDIO ===
// ===============================================

void configTimeNTP() {
    configTime(timeOffset, daylightOffset, ntpServer);
}

void actualizarNTP(struct timeval *t) {
    Serial.println("Hora NTP sincronizada con éxito.");
}

String obtenerHoraActual(bool conSegundos) {
    char buffer[10];
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        // Si falla la sincronización, muestra guiones
        return conSegundos ? "--:--:--" : "--:--"; 
    }
    const char* fmt = conSegundos ? "%H:%M:%S" : "%H:%M";
    strftime(buffer, sizeof(buffer), fmt, &timeinfo);
    return String(buffer);
}

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

void checkAlarma() {
    if (!alarmaActiva || modoActual != RELOJ) return;

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;

    if (timeinfo.tm_hour == alarmaHora && timeinfo.tm_min == alarmaMinuto && !alarmaCheckeada) {
        modoActual = ALARMA_SONANDO;
        alarmaCheckeada = true;
        Serial.println("!!! ALARMA DISPARADA !!!");
    } else if (timeinfo.tm_min != alarmaMinuto) {
        alarmaCheckeada = false;
    }
}


// ===============================================
// === 7. MANEJO DE BOTONES (3 Pasos FSM) ===
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

    Serial.printf("MODO: %d, ESTADO: %d | MODE:%d UP:%d DOWN:%d SET:%d\n", modoActual, alarmaSubEstado, mode, up, down, set);

    // 1. Manejar Alarma Sonando (HUSH)
    if (modoActual == ALARMA_SONANDO) {
        modoActual = RELOJ;
        noTone(BUZZER_PIN_SIGNAL);
        Serial.println("ALARMA SILENCIADA -> RELOJ");
        actualizarLCD();
        return;
    }
    
    // 2. Botón MODE (SALIDA RÁPIDA de la configuración o Entrada en RELOJ)
    if (mode) {
        if (modoActual == ALARMA_CONFIG) {
            modoActual = RELOJ; 
            Serial.println("ALARMA_CONFIG -> RELOJ (SALIDA MODE/RESET)");
            actualizarLCD();
            return; 
        } else if (modoActual == RELOJ) {
            modoActual = ALARMA_CONFIG;
            alarmaSubEstado = ALM_SET_HOUR;
            Serial.println("RELOJ -> ALARMA_CONFIG");
        }
    }
    
    // 3. FSM de Configuración de Alarma
    if (modoActual == ALARMA_CONFIG) {
        
        // UP/DOWN: Ajuste de Hora o Minuto
        int cambio = (up) ? 1 : ((down) ? -1 : 0);
        if (cambio != 0) {
            if (alarmaSubEstado == ALM_SET_HOUR) {
                alarmaHora = (alarmaHora + cambio + 24) % 24;
            } else if (alarmaSubEstado == ALM_SET_MINUTE) {
                alarmaMinuto = (alarmaMinuto + cambio + 60) % 60;
            }
        }
        
        // SET: Avance en los 2 pasos de ajuste
        if (set) {
            if (alarmaSubEstado == ALM_SET_HOUR) {
                alarmaSubEstado = ALM_SET_MINUTE;
                Serial.println("SET: Hora Guardada -> Ajustar Minuto");
            } else if (alarmaSubEstado == ALM_SET_MINUTE) {
                modoActual = RELOJ;
                Serial.println("SET: Minuto Guardado -> RELOJ");
            }
        }
    }

    actualizarLCD();
}


// ===============================================
// === 8. DISPLAY LCD ===
// ===============================================

void actualizarLCD() {
    bool parpadeando = (millis() / 500) % 2; 

    lcd.clear();
    lcd.setCursor(0, 0);

    if (modoActual == RELOJ) {
        String horaStr = obtenerHoraActual(true);
        
        lcd.print("RELOJ " + horaStr);
        lcd.setCursor(0, 1);
        lcd.printf("ALM %02d:%02d V:MAX %s", alarmaHora, alarmaMinuto, alarmaActiva ? "ON" : "OFF");
    } 
    
    else if (modoActual == ALARMA_CONFIG) { 
        
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

    // ********* LÓGICA DE CONEXIÓN WI-FI *********
    lcd.setCursor(0, 1);
    lcd.print("Conectando...");
    Serial.print("Conectando a Wi-Fi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    // Espera la conexión (el punto se irá moviendo en Serial)
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n¡Wi-Fi conectado! Sincronizando hora.");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Conectado! IP:");
        lcd.setCursor(0, 1);
        lcd.print(WiFi.localIP());
        
        // Una vez conectado, configura NTP
        sntp_set_time_sync_notification_cb(actualizarNTP);
        configTimeNTP();

    } else {
        Serial.println("\nError al conectar al Wi-Fi.");
        Serial.println("Revisa SSID/Password.");
        lcd.clear();
        lcd.print("ERROR WI-FI");
        lcd.setCursor(0, 1);
        lcd.print("Revisa credenciales");
    }
    // ********************************************

    delay(3000); // Tiempo para leer el mensaje de IP o error
    
    // Comenzar en RELOJ para ver si la hora se sincronizó
    modoActual = RELOJ;
    actualizarLCD();
}

void loop() {
    // 1. Procesar Botones (Siempre primero)
    manejarBotonesHardware();

    // 2. Lógica de Modos
    if (modoActual == RELOJ) {
        static time_t last_time = 0;
        time_t now = time(nullptr);

        if (now != last_time) {
            checkAlarma();
            actualizarLCD();
            last_time = now;
        }
    }

    else if (modoActual == ALARMA_SONANDO) {
        sonarAlarma(); 
        if ((millis() / 500) % 2) {
            actualizarLCD();
        }
    } 
    
    else if (modoActual == ALARMA_CONFIG) {
        static unsigned long lastUpdate = 0;
        if (millis() - lastUpdate >= 500) {
             actualizarLCD();
             lastUpdate = millis();
        }
    }
    
    delay(5);
}