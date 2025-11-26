# HUD e Interfaz de Usuario

## 1. Arquitectura del HUD (`src/hud`)

El Head-Up Display (HUD) sigue un diseño orientado a objetos polimórfico.

### Clase Base `Instrument`
Define la interfaz común para todos los indicadores:
```cpp
class Instrument {
public:
    virtual void render(Renderer2D& renderer, const FlightData& data) = 0;
    // Propiedades comunes: posición, tamaño, color, visibilidad
};
```

### Clase `FlightHUD`
Es el contenedor que gestiona la colección de instrumentos. En su método `render`, itera sobre todos los instrumentos activos y delega el dibujo.

## 2. Renderizado Vectorial (`Renderer2D`)

A diferencia de usar imágenes estáticas (sprites), el HUD se dibuja usando primitivas geométricas (líneas, rectángulos, triángulos).
*   **Ventaja**: Escalado infinito sin pixelación.
*   **Estilo**: Aspecto "retro" militar auténtico (como un F-16 real).

### Texto Procedural (`TextRenderer`)
El proyecto implementa una fuente de estilo "7 segmentos" dibujada proceduralmente.
*   No usa texturas de fuentes.
*   Cada carácter se construye dibujando pequeños rectángulos para los segmentos.
*   Esto es extremadamente eficiente en memoria y garantiza nitidez total.

## 3. Instrumentos Clave

### Pitch Ladder (Escalera de Cabeceo)
Es el instrumento más complejo matemáticamente.
*   Muestra líneas horizontales para el ángulo de pitch (cabeceo).
*   **Rotación**: Todo el conjunto debe rotar según el ángulo de roll (alabeo).
*   **Implementación**: Se usa una matriz de rotación 2D o trigonometría básica (`sin`, `cos`) para transformar cada punto de las líneas antes de dibujar.

### Tapes (Cintas) de Velocidad y Altitud
Muestran una "ventana" de valores que se desplaza verticalmente.
*   Se calcula qué rango de valores es visible (ej. altitud ± 200 pies).
*   Se dibujan las marcas (ticks) y números que caen dentro de ese rango, desplazados proporcionalmente.

### HSI (Horizontal Situation Indicator)
Muestra la navegación.
*   **Rosa de los Vientos**: Rota opuestamente al rumbo (Heading) del avión.
*   **Aguja de Waypoint**: Apunta al objetivo relativo al rumbo actual.

## 4. Planificador de Misiones (`MissionPlanner`)

Es una herramienta 2D completa dentro del simulador.
*   **Transformación de Coordenadas**: Convierte coordenadas del mundo (Metros X, Z) a coordenadas de pantalla (Píxeles X, Y) para dibujar el mapa y los waypoints.
*   **Interacción**: Detecta clics del mouse sobre los iconos de waypoints para arrastrarlos.

## 5. Puntos Clave para el Examen

*   **Polimorfismo**: Cómo `FlightHUD` trata a todos los instrumentos por igual.
*   **Sistemas de Coordenadas 2D**: Origen arriba-izquierda (pantalla) vs abajo-izquierda (OpenGL clásico) vs centro (HUD).
*   **Matrices de Transformación 2D**: Rotación y traslación de primitivas para el Pitch Ladder y HSI.
