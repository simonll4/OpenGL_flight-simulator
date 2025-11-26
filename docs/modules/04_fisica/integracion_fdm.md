# Física de Vuelo e Integración FDM

## 1. Modelo de Dinámica de Vuelo (FDM)

El simulador utiliza un solver de física de 6 Grados de Libertad (6-DoF) encapsulado en `flight::FdmSimulation`. Este modelo calcula las fuerzas y momentos aerodinámicos que actúan sobre el avión basándose en:
*   Coeficientes aerodinámicos (Lift, Drag, Momentos).
*   Estado actual (Velocidad, Ángulo de Ataque, Deslizamiento Lateral).
*   Entradas de control (Alerones, Elevador, Timón, Potencia).

## 2. El Controlador (`FlightSimulationController`)

Esta clase actúa como intermediario entre el sistema de entrada (GLFW) y el núcleo físico.

### Joystick Virtual
Dado que un teclado es digital (tecla presionada o no), se implementa un **Joystick Virtual** para simular controles analógicos.
*   **Suavizado Exponencial**: Las entradas no cambian instantáneamente de 0 a 1. Se mueven gradualmente hacia el valor objetivo.
    ```cpp
    current = current + (target - current) * responseRate * dt;
    ```
    Esto evita movimientos espasmódicos y simula la inercia de los actuadores hidráulicos.

## 3. Sistemas de Coordenadas

Uno de los desafíos principales es la conversión entre el sistema de coordenadas de la física y el de renderizado.

*   **Física (NED)**: North-East-Down.
    *   +X: Norte
    *   +Y: Este
    *   +Z: Abajo
*   **OpenGL (ENU/World)**: Right-Handed, Y-Up.
    *   +X: Este
    *   +Y: Arriba
    *   +Z: Sur (hacia la cámara)

El `FlightSimulationController` realiza esta conversión antes de actualizar la posición del modelo 3D.

## 4. Estructura `FlightData`

Para desacoplar la física del HUD, se utiliza una estructura de datos intermedia `flight::FlightData`. Esta contiene toda la telemetría necesaria en unidades amigables para la aviación:
*   **Altitud**: Pies (ft).
*   **Velocidad**: Nudos (kt).
*   **Velocidad Vertical**: Pies por minuto (ft/min).
*   **Ángulos**: Grados (deg) para Pitch, Roll, Heading.

Esta estructura es la "Fuente de Verdad" para todos los instrumentos del HUD.

## 5. Puntos Clave para el Examen

*   **Integración Numérica**: El solver avanza el estado físico en pasos de tiempo discretos (`dt`).
*   **Normalización de Inputs**: Los controles se mapean al rango [-1, 1] antes de enviarse al solver.
*   **Conversión de Unidades**: Importancia de distinguir entre unidades internas (SI: metros, m/s, radianes) y de visualización (Imperial: pies, nudos, grados).
