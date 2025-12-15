## Estructura del Código: Archivos de Implementación (`src`)



La carpeta `src` contiene la implementación funcional (lógica de programación) de cada uno de los gestores del sistema, asegurando que el microcontrolador ESP32 interactúe correctamente con los periféricos.

| Archivo (`.cpp`) | Contenido Principal (Implementación) |
| :--- | :--- |
| **`main.cpp`** | **El Punto de Entrada:** Este archivo contiene las funciones `setup()` y `loop()` del ESP32. Se encarga de crear las instancias de todos los gestores (`CronoMgr crono;`, `DisplayMgr display;`, etc.) y de llamar sus métodos de actualización cíclicamente, coordinando la operación de todo el sistema. |
| **`CronoMgr.cpp`** | Contiene el código de inicialización del `RtcDS1302`, las llamadas de lectura y escritura del tiempo, y el manejo de la batería de respaldo del RTC. |
| **`DisplayMgr.cpp`** | Contiene la inicialización de `MD_Parola`, las funciones que gestionan la lectura analógica de `LDRPIN` (`analogRead()`), y el mapeo de estos valores al brillo (`setIntensity()`) de los MAX7219. |
| **`AlarmMgr.cpp`** | Implementación de las llamadas a la función `tone()` en el pin del buzzer, la melodía de la alarma y la lógica de estado (encendido/apagado). |
| **`InputMgr.cpp`** | Implementación de la lectura de los pines de botón y el algoritmo de *debounce* que asegura que una sola presión de botón se interprete como una única acción. |
| **`TimerMgr.cpp`** | Implementación de las estructuras de control de tiempo usando `millis()`, creando temporizadores virtuales que no detienen el flujo del programa, manteniendo la eficiencia del sistema. |
| **`WiFiMgr.cpp`** | Contiene las rutinas para conectarse a una red WiFi (ej. `WiFi.begin()`), la gestión del servidor NTP y las rutinas de reconexión automática. |