
![Diagrama de Conexión del Reloj](Anexos/CIRCUITO.png)

# ⌚ Reloj Digital Multifunción MAX7219 con ESP32 y Wi-Fi

Este repositorio alberga el firmware para un proyecto de reloj digital avanzado y multifuncional, diseñado para ser ejecutado en la placa de desarrollo **ESP32 DevKit V1**. El proyecto supera la función básica de un reloj al integrar modos de operación avanzados, conectividad inalámbrica y una interfaz de usuario completa. La arquitectura del software está diseñada para ser modular y escalable.

## 🚀 Características Principales del Proyecto

| Característica | Descripción | Componentes Relevantes |
| :--- | :--- | :--- |
| **Reloj en 24 Horas** | Muestra la hora precisa (HH:MM:SS) en formato de 24 horas. | MAX7219, RTC DS102, NTP. |
| **Sincronización NTP** | Utiliza el MODO Wi-Fi para obtener y corregir la hora del \textbf{RTC DS102} mediante el Network Time Protocol. | ESP32, Conexión Wi-Fi. |
| **Cronómetro y Temporizador** | Funciones independientes para medir o contar regresivamente el tiempo. Controlables vía botones o interfaz web. | ESP32 (Temporizadores internos). |
| **Sistema de Alarma** | Permite configurar una hora de alarma persistente, que activa el \textbf{Buzzer HW405} al coincidir con la hora actual. | RTC DS102, Buzzer HW405, Memoria NVS/EEPROM. |
| **Menú de Configuración** | Interfaz local mediante botones y la \textbf{Pantalla LCD I2C} para la configuración de modos, alarmas y Wi-Fi. | Pantalla LCD I2C, Botones. |
| **Control Remoto (Wi-Fi)** | El ESP32 actúa como un Servidor Web, permitiendo la configuración remota de la alarma y el control del cronómetro desde un navegador. | ESP32 (Servidor HTTP). |

## 🛠️ Componentes de Hardware Requeridos

Este proyecto requiere los siguientes módulos de hardware para su implementación completa:

* **Microcontrolador:** ESP32 DevKit V1 (Cerebro del sistema y conectividad Wi-Fi).
* **Display Principal:** Módulo de display de 7 segmentos o matriz de LEDs controlado por el chip **MAX7219** (Comunicación SPI).
* **Reloj de Tiempo Real (RTC):** Módulo \textbf{DS102} (Mantiene la hora precisa y persistente).
* **Interfaz de Usuario:** Pantalla \textbf{LCD I2C} (Para el menú y la configuración).
* **Audio:** \textbf{Buzzer HW405} (Para notificaciones de alarma).
* **Entrada:** Botones físicos y Fuente de alimentación.



## 💻 Estructura del Software y Modularidad

El código sigue una estructura modular para facilitar su mantenimiento y comprensión, basada en la separación de interfaces y su implementación: 

### 1. Relación entre Carpetas `include` y `src`

| Carpeta | Contenido | Función en el Proyecto |
| :--- | :--- | :--- |
| **`include`** | Archivos de Encabezado (`.h`, `.hpp`). | Define las **interfaces (declaraciones)** de las funciones, estructuras y constantes utilizadas (ej. configuración de pines, definición de la clase `Alarm`). |
| **`src`** | Archivos de Implementación (`.cpp`). | Contiene la **lógica funcional (implementación)** real. Aquí se define cómo interactuar con el MAX7219, la lógica de los modos de cronómetro, y la gestión del servidor web. |

Esta estructura asegura que el archivo principal de ejecución (`main.cpp`) solo necesite conocer las interfaces, delegando los detalles de bajo nivel a los archivos en `src`.

### 2. Lógica de Modos Operativos

El núcleo del firmware gestiona los diferentes modos operativos a través de una **Máquina de Estados Finita (FSM)**.

#### 2.1. Modo de Reloj y Sincronización

1.  **Obtención de Tiempo:** El sistema prioriza la lectura del \textbf{RTC DS102}.
2.  **Sincronización:** Si el Wi-Fi está activo, se realiza una consulta al servidor **NTP**. Si la consulta es exitosa, se utiliza esa hora de alta precisión para **corregir y actualizar el RTC**.

#### 2.2. Modo de Alarma

La lógica de comparación para la activación de la alarma es:

$$\text{IF } (H_{actual} = H_{alarma}) \land (M_{actual} = M_{alarma}) \text{ THEN ACTIVATE BUZZER}$$

#### 2.3. MODO Wi-Fi y Control Remoto

El modo Wi-Fi permite la sincronización NTP y actúa como un **Servidor HTTP** accesible en la red local . Esto proporciona una interfaz remota para la configuración y el control de las funciones del reloj.

## ⚙️ Implementación del Display (MAX7219)

El control del display se realiza utilizando una librería adecuada para el **MAX7219** (como `LedControl.h`).

* **Comunicación:** El ESP32 se comunica con el MAX7219 a través de la interfaz serial de 3 pines (\texttt{DIN}, \texttt{CLK}, \texttt{LOAD}).
* **Lógica:** El código es responsable de tomar los valores de la hora y transformarlos en los comandos que el MAX7219 necesita para encender los segmentos o LEDs correspondientes.

```cpp
#include "LedControl.h"
// Conexión Típica: (DIN, CLK, LOAD, Número de matrices)
LedControl lc = LedControl(DIN_PIN, CLK_PIN, LOAD_PIN, 1);