Carpeta src (Archivos de Implementación .cpp)
La carpeta src contiene la implementación concreta del código para cada una de las clases declaradas en la carpeta include. Es aquí donde las funciones son definidas en detalle, incluyendo la lógica de procesamiento, las llamadas a librerías de bajo nivel y las interacciones con el hardware del ESP32.

Archivo (.cpp),Contenido Principal (Implementación)
AlarmMgr.cpp,"Implementación de las llamadas a la función tone() en el pin del buzzer, la melodía de la alarma y la lógica de estado (encendido/apagado)."
CronoMgr.cpp,"Contiene el código de inicialización del RtcDS1302, las llamadas de lectura y escritura del tiempo, y el manejo de la batería de respaldo del RTC."
DisplayMgr.cpp,"Contiene la inicialización de MD_Parola, las funciones que gestionan la lectura analógica de LDRPIN (analogRead()), y el mapeo de estos valores al brillo (setIntensity()) de los MAX7219."
InputMgr.cpp,Implementación de la lectura de los pines de botón y el algoritmo de debounce que asegura que una sola presión de botón se interprete como una única acción.
TimerMgr.cpp,"Implementación de las estructuras de control de tiempo usando millis(), creando temporizadores virtuales que no detienen el flujo del programa."
WiFiMgr.cpp,"Contiene las rutinas para conectarse a una red WiFi (ej. WiFi.begin()), la gestión del servidor NTP y las rutinas de reconexión."
main.cpp,"El Punto de Entrada (equivalente a setup() y loop()): Este archivo es el más importante, ya que contiene el bucle principal del programa. Aquí se crean las instancias de todos los gestores (CronoMgr crono;, DisplayMgr display;, etc.) y se llaman sus métodos de actualización cíclicamente, coordinando la operación de todo el sistema."