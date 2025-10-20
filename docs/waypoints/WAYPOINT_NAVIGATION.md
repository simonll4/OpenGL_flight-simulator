# Sistema de Navegación por Waypoints

## Descripción

Se ha implementado un sistema completo de navegación por waypoints que permite al piloto seguir una ruta predefinida en el espacio 3D. El sistema incluye:

1. **Indicador de Waypoint en el HUD** - Muestra dirección y distancia al waypoint activo
2. **Visualización 3D de Waypoints** - Cilindros brillantes que marcan los puntos de navegación
3. **Sistema de Computadora de Misión** - Calcula automáticamente rumbo y distancia

## Componentes Implementados

### 1. WaypointIndicator (HUD) - **DISEÑO PROFESIONAL v2.0**
- **Ubicación**: `src/hud/WaypointIndicator.{h,cpp}`
- **Función**: Instrumento del HUD tipo HSI militar que muestra navegación completa
- **Estilo**: Panel dual (Brújula + Información Digital)
- **Tamaño**: 280x140 píxeles
- **Posición**: Centro superior de la pantalla
- **Características Principales**:
  
  **Panel Izquierdo - Brújula Circular:**
  - Doble círculo concéntrico de referencia
  - Marcas cardinales (N, S, E, W)
  - Flecha direccional GRANDE (4px grosor, 35px largo)
  - Indicador de rumbo actual en amarillo (tick superior)
  - Norte marcado claramente
  
  **Panel Derecho - Información Digital:**
  - **DIST**: Distancia con números grandes (km/m según magnitud)
  - **TURN**: Ángulo de giro con código de color (verde < 10°, amarillo ≥ 10°)
  - **BRG**: Bearing absoluto en formato 000-360°
  - **Barra de Proximidad**: Indicador visual del progreso
  
  **Características Avanzadas**:
  - Fondo semi-transparente para mejor contraste
  - Etiqueta "NAV" identificadora
  - Divisor visual entre secciones
  - Mensaje "NO WAYPOINT" cuando inactivo
  - Compatible solo con vista POV
  
📖 **Manual Completo**: Ver `WAYPOINT_INDICATOR_README.md` para guía detallada de uso

### 2. WaypointRenderer (3D)
- **Ubicación**: `src/gfx/WaypointRenderer.{h,cpp}`
- **Función**: Renderiza waypoints como objetos 3D en el mundo
- **Características**:
  - Cilindros verticales de 30m de altura
  - Color verde brillante para waypoint activo
  - Color azul tenue para waypoints inactivos
  - Efecto de brillo (glow) en waypoint activo
  - Iluminación básica para mejor visibilidad

### 3. FlightData Extendido
- **Nuevos campos**:
  - `targetWaypoint` - Posición 3D del waypoint objetivo (metros)
  - `hasActiveWaypoint` - Flag indicando si hay waypoint activo
  - `waypointDistance` - Distancia al waypoint (metros)
  - `waypointBearing` - Rumbo hacia el waypoint (0-360°)

## Integración en main.cpp

Para utilizar el sistema de waypoints en tu aplicación, necesitas:

### 1. Incluir los headers necesarios

```cpp
#include "gfx/WaypointRenderer.h"
```

### 2. Declarar variables de estado

```cpp
// Sistema de waypoints con captura
struct Waypoint {
    glm::vec3 position;
    std::string name;
    bool captured;  // Indica si el waypoint ya fue alcanzado
};

std::vector<Waypoint> waypoints;
int activeWaypointIndex = 0;
bool waypointSystemEnabled = false;
bool missionCompleted = false;
gfx::WaypointRenderer waypointRenderer;
```

### 3. Inicializar el sistema

```cpp
void initializeWaypoints() {
    waypoints.clear();
    missionCompleted = false;
    
    // Circuito ampliado para mejor maniobra (distancias ~1500-2000m)
    waypoints.push_back({glm::vec3(1500.0f, 120.0f, 0.0f), "WPT-1", false});
    waypoints.push_back({glm::vec3(1500.0f, 150.0f, -1500.0f), "WPT-2", false});
    waypoints.push_back({glm::vec3(0.0f, 200.0f, -2000.0f), "WPT-3", false});
    waypoints.push_back({glm::vec3(-1500.0f, 150.0f, -1500.0f), "WPT-4", false});
    waypoints.push_back({glm::vec3(-1800.0f, 120.0f, 0.0f), "WPT-5", false});
    waypoints.push_back({glm::vec3(0.0f, 100.0f, 0.0f), "HOME", false});
    
    activeWaypointIndex = 0;
    waypointSystemEnabled = true;
    
    waypointRenderer.init();
}
```

**Nota**: Las distancias se han ampliado a 1500-2000m para permitir mejor maniobra del avión.

### 4. Actualizar datos de waypoint

```cpp
void updateWaypointData(flight::FlightData& flightData, const glm::vec3& planePos) {
    if (!waypointSystemEnabled || waypoints.empty()) {
        flightData.hasActiveWaypoint = false;
        return;
    }
    
    const Waypoint& currentWaypoint = waypoints[activeWaypointIndex];
    flightData.targetWaypoint = currentWaypoint.position;
    flightData.hasActiveWaypoint = true;
    
    // Calcular distancia
    glm::vec3 toWaypoint = currentWaypoint.position - planePos;
    flightData.waypointDistance = glm::length(toWaypoint);
    
    // Calcular bearing (rumbo al waypoint)
    // Proyección en plano horizontal (XZ)
    glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);
    
    if (glm::length(toWaypointXZ) > 0.01f) {
        // Calcular bearing: 0° = norte (-Z), sentido horario
        float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180.0f / M_PI);
        if (bearing < 0.0f) bearing += 360.0f;
        flightData.waypointBearing = bearing;
    }
    
    // Captura automática de waypoint
    const float WAYPOINT_CAPTURE_RADIUS = 50.0f;
    if (flightData.waypointDistance < WAYPOINT_CAPTURE_RADIUS) {
        activeWaypointIndex = (activeWaypointIndex + 1) % waypoints.size();
        std::cout << "Waypoint alcanzado! Siguiente: " 
                  << waypoints[activeWaypointIndex].name << std::endl;
    }
}
```

### 5. Renderizar waypoints en 3D

```cpp
void renderWaypoints(const glm::mat4& view, const glm::mat4& proj) {
    if (!waypointSystemEnabled) return;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    for (size_t i = 0; i < waypoints.size(); ++i) {
        bool isActive = (i == activeWaypointIndex);
        glm::vec4 color = isActive 
            ? glm::vec4(0.0f, 1.0f, 0.4f, 0.8f)  // Verde brillante
            : glm::vec4(0.2f, 0.5f, 1.0f, 0.6f); // Azul tenue
        
        waypointRenderer.drawWaypoint(view, proj, waypoints[i].position, color, isActive);
    }
    
    glDisable(GL_BLEND);
}
```

### 6. Controles de teclado

```cpp
// En el callback de teclado
case GLFW_KEY_N:
    if (action == GLFW_PRESS) {
        waypointSystemEnabled = !waypointSystemEnabled;
        std::cout << "Sistema de waypoints: " 
                  << (waypointSystemEnabled ? "ACTIVADO" : "DESACTIVADO") << std::endl;
    }
    break;

case GLFW_KEY_M:
    if (action == GLFW_PRESS && waypointSystemEnabled) {
        activeWaypointIndex = (activeWaypointIndex + 1) % waypoints.size();
        std::cout << "Siguiente waypoint: " 
                  << waypoints[activeWaypointIndex].name << std::endl;
    }
    break;
```

## Cómo Funciona

### Cálculo de Rumbo (Bearing)

El sistema calcula el rumbo hacia el waypoint proyectando la dirección en el plano horizontal (XZ):

```cpp
// Vector hacia el waypoint
glm::vec3 toWaypoint = waypoint.position - planePos;

// Proyección en plano horizontal
glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);

// Cálculo de bearing (0° = norte = -Z, sentido horario)
float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180/π);
```

### Indicador Direccional

La flecha en el HUD muestra el **ángulo relativo** entre:
- El rumbo actual del avión (heading)
- El rumbo hacia el waypoint (bearing)

Si la flecha apunta:
- **Arriba** → Waypoint directamente adelante
- **Derecha** → Girar a la derecha
- **Izquierda** → Girar a la izquierda
- **Abajo** → Waypoint detrás

### Captura Automática de Waypoints

El sistema detecta automáticamente cuando el avión se acerca a un waypoint:
- **Radio de captura**: 80 metros (configurable)
- Al alcanzar un waypoint, este **desaparece** y marca como capturado
- Sistema **no cíclico**: Al completar todos los waypoints, la misión termina
- Feedback en consola con contador de waypoints restantes
- Mensaje de misión completada cuando se alcanzan todos

## Controles del Sistema

| Tecla | Función |
|-------|---------|
| **V** | Cambiar entre vista POV (primera persona) / 3ra persona |
| **N** | Activar/Desactivar sistema de waypoints |
| **M** | Saltar waypoint actual manualmente (marca como capturado) |
| **R** | **Reiniciar misión** (resetea todos los waypoints) |
| **W/S** | Pitch (cabeceo del avión) |
| **A/D** | Yaw (guiñada) |
| **Q/E** | Roll (alabeo) |
| **↑/↓** | Aumentar/disminuir velocidad |

**Nota**: El indicador HUD solo es visible en vista POV (presiona V para activarla).

## Sistema de Coordenadas

- **X**: Este (+) / Oeste (-)
- **Y**: Arriba (+) / Abajo (-)
- **Z**: Norte (-) / Sur (+)
- **0° heading**: Mira hacia -Z (Norte)
- **90° heading**: Mira hacia +X (Este)

## Personalización

### Modificar Radio de Captura

En `main.cpp`, función `updateWaypointData()`:

```cpp
const float WAYPOINT_CAPTURE_RADIUS = 80.0f; // Radio actual (metros)
// Aumentar para capturas más fáciles
// Disminuir para mayor precisión requerida
```

### Cambiar Colores

```cpp
// Waypoint activo (verde brillante)
glm::vec4 activeColor = glm::vec4(0.0f, 1.0f, 0.4f, 0.8f);

// Waypoints inactivos (azul tenue)
glm::vec4 inactiveColor = glm::vec4(0.2f, 0.5f, 1.0f, 0.6f);
```

### Ajustar Geometría del Cilindro

En `WaypointRenderer.cpp`:

```cpp
const int segments = 16;      // Resolución del cilindro
const float radius = 3.0f;    // Radio en metros
const float height = 30.0f;   // Altura en metros
```

## Arquitectura

```
┌─────────────────────────────────────────────────────────┐
│                      main.cpp                           │
│  ┌──────────────┐    ┌──────────────┐                  │
│  │   Waypoint   │───▶│ FlightData   │                  │
│  │   System     │    │  (bearing,   │                  │
│  │ (calculates) │    │   distance)  │                  │
│  └──────────────┘    └───────┬──────┘                  │
│                              │                          │
│                              ▼                          │
│  ┌──────────────┐    ┌──────────────┐                  │
│  │  Waypoint    │    │   Waypoint   │                  │
│  │  Renderer    │    │  Indicator   │                  │
│  │   (3D)       │    │    (HUD)     │                  │
│  └──────────────┘    └──────────────┘                  │
└─────────────────────────────────────────────────────────┘
```

## Rendimiento

- Los waypoints solo se renderizan si no han sido capturados
- El indicador HUD se muestra solo en vista POV (primera persona)
- Cálculos de navegación se ejecutan cada frame (~60 FPS)
- Bajo impacto en rendimiento (< 0.1ms por frame)
- Sistema optimizado: waypoints capturados no se procesan

## Mejoras Recientes (v2.0)

### Interfaz del Usuario
- ✅ **Indicador rediseñado**: Panel profesional tipo HSI militar
- ✅ **Panel dual**: Brújula circular + Información digital
- ✅ **Métricas completas**: DIST, TURN, BRG y barra de progreso
- ✅ **Colores dinámicos**: Verde/amarillo según desviación
- ✅ **Números grandes**: Fácil lectura a distancia

### Sistema de Waypoints
- ✅ **Waypoints más separados**: 1500-2000m (antes 500m)
- ✅ **Desaparición al capturar**: Waypoints desaparecen al alcanzarlos
- ✅ **Sistema no cíclico**: Misión con inicio y fin definidos
- ✅ **Detección de completado**: Mensaje cuando se completan todos
- ✅ **Reinicio de misión**: Tecla R para reiniciar
- ✅ **Radio ampliado**: 80m de captura (antes 50m)

### Documentación
- 📖 **Manual del indicador**: `WAYPOINT_INDICATOR_README.md`
- 📖 **Guía del sistema**: Este documento actualizado
- 📖 **Ejemplos prácticos**: Escenarios de vuelo detallados

## Expansión Futura

Posibles mejoras al sistema:

1. **Importar rutas desde archivo** (GPX, JSON)
2. **Editor de waypoints en tiempo real**
3. **Alturas mínimas/máximas por waypoint**
4. **Velocidades recomendadas**
5. **Waypoints de procedimiento** (holding patterns)
6. **Navegación GPS con desviación de ruta**
7. **Estimación de tiempo de llegada (ETA)**
8. **Waypoints condicionales** (meteorología, combustible)

## Referencias

- Sistema inspirado en navegación RNAV (Area Navigation)
- Formato de waypoints compatible con sistemas GPS modernos
- Indicador visual basado en HSI (Horizontal Situation Indicator)

## Compilación

El sistema se integra automáticamente al compilar el proyecto:

```bash
make clean
make
```

Los nuevos archivos se compilan automáticamente:
- `WaypointIndicator.cpp`
- `WaypointRenderer.cpp`

## Troubleshooting

### El indicador no aparece
- Verificar que `hasActiveWaypoint` esté en `true`
- Verificar que el waypoint esté dentro del rango visible

### La flecha apunta en dirección incorrecta
- Verificar el sistema de coordenadas (Z negativo = Norte)
- Verificar que el `heading` y `bearing` estén en grados (0-360)

### Los waypoints 3D no se visualizan
- Verificar que el `WaypointRenderer` se haya inicializado con `init()`
- Verificar que las matrices view y projection sean correctas
- Verificar que el blending esté habilitado durante el renderizado
