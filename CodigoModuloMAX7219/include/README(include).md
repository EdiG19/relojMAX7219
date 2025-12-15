Descripción de los Módulos de Cabecera (Archivos .h)
El proyecto del reloj digital utiliza una estructura modular donde cada archivo de cabecera (.h) declara la interfaz de un componente específico.

Módulos de Núcleo y Sincronización
El módulo CronoMgr.h (Gestor de Cronometraje) es el más central del sistema. Su propósito es declarar la clase que gestiona la lectura, inicialización y mantenimiento de la hora y fecha a través del módulo RTC (DS1302). Es la principal fuente de información de tiempo (hour, minute, second) consumida por casi todos los demás gestores funcionales. Mantiene la comunicación a bajo nivel con el DS1302 usando el protocolo Three-Wire, lo que asegura que la hora se mantenga precisa incluso sin la energía principal (gracias a la batería del RTC).

Para la configuración del entorno, GlobalSettings.h (Configuración Global) contiene constantes, enumeraciones y definiciones esenciales para el hardware, como los pines GPIO utilizados para las conexiones SPI, I2C, la LDR y el Buzzer. Este archivo es fundamental, ya que es incluido por la mayoría de los módulos para establecer la conexión física, y define la arquitectura del hardware (ej., el número de matrices MAX7219 encadenadas) y las frecuencias del sistema si es necesario.

Módulos de Interfaz y Visualización
DisplayMgr.h (Gestor de Pantalla) declara la clase responsable de controlar las matrices MAX7219 y dar formato a la información mostrada (hora, alarmas, mensajes de estado). Requiere datos de CronoMgr.h y AlarmMgr.h. Se comunica con el hardware del MAX7219 a través de la librería SPI (MD_Parola). Incorpora la lógica de control de brillo automático, leyendo el valor de la LDR y ajustando la intensidad de los LEDs a través del método setIntensity() de la librería MAX7219.

Para la interacción, InputMgr.h (Gestor de Entradas) declara la clase que maneja la interacción del usuario a través de botones (físicos o táctiles), gestionando la detección de pulsaciones cortas, largas y dobles. Sus métodos son utilizados por CronoMgr.h y AlarmMgr.h para configurar y manipular la hora y el estado de la alarma. Incluye la implementación del debounce (antirrebote) para asegurar lecturas de botón estables y confiables.

Módulos de Funcionalidad Adicional
El módulo AlarmMgr.h (Gestor de Alarmas) declara la clase que encapsula toda la lógica de la alarma, incluyendo la verificación de la hora, el estado de activación y el manejo del sonido. Depende de CronoMgr.h para obtener el tiempo actual y de InputMgr.h para que el usuario pueda configurarla o desactivarla. Usualmente incluye un método checkAlarm() que es llamado en el bucle principal (main.cpp).

TimerMgr.h (Gestor de Temporizadores) declara la clase que proporciona temporizadores no bloqueantes basados en el tiempo del sistema (millis() o micros()). Es utilizado internamente para controlar la frecuencia de actualización de la pantalla, la duración del tono del buzzer, o los retrasos necesarios en las comunicaciones de red. Su propósito clave en sistemas embebidos es evitar el uso de la función delay(), permitiendo que el bucle principal (main.cpp) se ejecute continuamente.

Finalmente, WiFiMgr.h (Gestor WiFi) declara la clase que maneja la conectividad WiFi, la reconexión y, posiblemente, la sincronización de la hora a través del protocolo NTP (Network Time Protocol). Si está activo, utiliza CronoMgr.h para actualizar el RTC con una hora precisa de internet. Puede incluir lógica para crear un portal de configuración (Access Point) en caso de que la conexión principal falle.
