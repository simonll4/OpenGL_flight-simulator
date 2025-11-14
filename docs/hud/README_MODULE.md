# Módulo HUD - Documentación de Arquitectura

## Visión General

El módulo HUD (Heads-Up Display) es el sistema de interfaz de usuario para el simulador de vuelo. Proporciona instrumentos visuales en tiempo real que muestran información crítica de vuelo sin que el piloto tenga que apartar la vista del mundo 3D.

---

## Estructura del Módulo (Organizada)

```
src/hud/
├── core/                           # Sistema base del HUD
│   ├── Instrument.h                # Clase base abstracta
│   ├── Instrument.cpp              # Implementación base
│   ├── FlightHUD.h                 # Coordinador central
│   └── FlightHUD.cpp               # Gestión de instrumentos
│
├── instruments/                    # Instrumentos implementados
│   ├── flight/                     # Parámetros de vuelo básicos
│   │   ├── Altimeter.h             # Indicador de altitud
│   │   ├── Altimeter.cpp
│   │   ├── SpeedIndicator.h        # Indicador de velocidad
│   │   └── SpeedIndicator.cpp
│   │
│   └── navigation/                 # Sistemas de navegación
│       ├── WaypointIndicator.h     # Navegación por waypoints
│       └── WaypointIndicator.cpp
│
└── docs/                           # Documentación
    └── README_INSTRUMENTS.md       # Guía de instrumentos
```

---

## Organización por Categorías

### 1. **core/** - Sistema Base

Contiene las clases fundamentales del sistema HUD.

#### **Instrument** (Clase Base Abstracta)
- **Propósito**: Define la interfaz común para todos los instrumentos
- **Responsabilidades**:
  - Gestión de propiedades comunes (posición, tamaño, color)
  - Interfaz de renderizado abstracta
  - Control de visibilidad (enable/disable)
  
**Archivo**: `core/Instrument.{h,cpp}`

```cpp
class Instrument {
protected:
    glm::vec2 position_;    // Posición en pantalla
    glm::vec2 size_;        // Dimensiones del instrumento
    glm::vec4 color_;       // Color principal
    bool enabled_;          // Estado de visibilidad
    
public:
    virtual void render(Renderer2D&, const FlightData&) = 0;  // Abstracto
};
```

#### **FlightHUD** (Coordinador Central)
- **Propósito**: Gestiona todos los instrumentos como un sistema unificado
- **Responsabilidades**:
  - Crear e inicializar instrumentos
  - Coordinar el renderizado en orden
  - Gestionar layouts (posicionamiento)
  - Manejar cambios de resolución

**Archivo**: `core/FlightHUD.{h,cpp}`

```cpp
class FlightHUD {
private:
    std::vector<std::unique_ptr<Instrument>> instruments_;
    std::unique_ptr<Renderer2D> renderer2D_;
    
public:
    void init(int screenWidth, int screenHeight);
    void update(const FlightData& data);
    void render();
    void setLayout(const std::string& layoutName);
};
```

---

### 2. **instruments/flight/** - Parámetros de Vuelo

Instrumentos que muestran datos básicos de vuelo.

#### **Altimeter** (Altímetro)
- **Función**: Muestra la altitud del avión en pies
- **Estilo**: Tape vertical con escala móvil + display digital
- **Ubicación**: Lado derecho del HUD
- **Características**:
  - Escala cada 100 pies
  - Display de 7 segmentos
  - Auto-escala según altitud

**Archivo**: `instruments/flight/Altimeter.{h,cpp}`

**Estado**: ✅ Implementado y funcional

#### **SpeedIndicator** (Indicador de Velocidad)
- **Función**: Muestra la velocidad aerodinámica en nudos
- **Estilo**: Tape vertical similar al altímetro
- **Ubicación**: Lado izquierdo del HUD (cuando se active)
- **Características**:
  - Escala cada 10 nudos
  - Indicador de velocidad objetivo
  - Zonas de color según rango

**Archivo**: `instruments/flight/SpeedIndicator.{h,cpp}`

**Estado**: ⚠️ Implementado pero deshabilitado (pendiente activación)

---

### 3. **instruments/navigation/** - Sistemas de Navegación

Instrumentos para ayuda en navegación y guía de ruta.

#### **WaypointIndicator** (Indicador de Waypoints)
- **Función**: Guía visual para navegación por waypoints
- **Estilo**: Panel dual HSI militar profesional
- **Ubicación**: Centro superior del HUD
- **Características**:
  - Brújula circular con flecha direccional
  - Información digital (DIST, TURN, BRG)
  - Barra de proximidad
  - Código de color dinámico

**Archivo**: `instruments/navigation/WaypointIndicator.{h,cpp}`

**Estado**: ✅ Implementado v2.0 - Diseño profesional

📖 **Documentación completa**: `WAYPOINT_INDICATOR_README.md` (raíz del proyecto)

---

## Principios de Diseño

### 1. **Separación de Responsabilidades**

Cada categoría tiene un propósito claro:
- **core/**: Funcionalidad base reutilizable
- **instruments/flight/**: Datos de vuelo inmediatos
- **instruments/navigation/**: Guía y planificación de ruta

### 2. **Polimorfismo y Extensibilidad**

Todos los instrumentos heredan de `Instrument`:
```cpp
// Fácil agregar nuevos instrumentos
class AttitudeIndicator : public Instrument {
    void render(Renderer2D&, const FlightData&) override;
};
```

### 3. **Gestión Centralizada**

`FlightHUD` maneja todo mediante contenedores polimórficos:
```cpp
std::vector<std::unique_ptr<Instrument>> instruments_;

// Renderizado uniforme
for (auto& instrument : instruments_) {
    if (instrument->isEnabled()) {
        instrument->render(renderer, flightData);
    }
}
```

### 4. **Escalabilidad**

Estructura preparada para crecimiento:
```
instruments/
├── flight/           # Parámetros básicos
│   ├── Altimeter
│   ├── SpeedIndicator
│   ├── AttitudeIndicator   (futuro)
│   └── VSI                 (futuro)
│
├── navigation/       # Navegación y guía
│   ├── WaypointIndicator
│   ├── HeadingIndicator    (futuro)
│   └── HSI                 (futuro)
│
├── engine/           (futuro)
│   ├── FuelGauge
│   ├── RPMIndicator
│   └── TempGauge
│
└── systems/          (futuro)
    ├── WarningPanel
    ├── RadioAltimeter
    └── GPWS
```

---

## Cómo Agregar un Nuevo Instrumento

### Paso 1: Decidir Categoría

¿El instrumento muestra datos de vuelo básicos o navegación?
- **Vuelo básico** → `instruments/flight/`
- **Navegación** → `instruments/navigation/`
- **Motor/sistemas** → crear nueva categoría

### Paso 2: Crear Archivos

**Ejemplo**: Agregar un AttitudeIndicator (Horizonte Artificial)

```bash
cd src/hud/instruments/flight/
touch AttitudeIndicator.h AttitudeIndicator.cpp
```

### Paso 3: Implementar Clase

**AttitudeIndicator.h**:
```cpp
#pragma once
#include "../../core/Instrument.h"

namespace hud {
    class AttitudeIndicator : public Instrument {
    public:
        AttitudeIndicator();
        void render(gfx::Renderer2D&, const flight::FlightData&) override;
    
    private:
        void drawHorizonLine(gfx::Renderer2D&, float pitch, float roll);
        void drawPitchLadder(gfx::Renderer2D&, float pitch);
    };
}
```

**AttitudeIndicator.cpp**:
```cpp
#include "AttitudeIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"

namespace hud {
    AttitudeIndicator::AttitudeIndicator() : Instrument() {
        size_ = glm::vec2(300.0f, 300.0f);  // Cuadrado
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }
    
    void AttitudeIndicator::render(gfx::Renderer2D& renderer, 
                                     const flight::FlightData& flightData) {
        drawHorizonLine(renderer, flightData.pitch, flightData.roll);
        drawPitchLadder(renderer, flightData.pitch);
    }
    
    // ... implementación de métodos privados
}
```

### Paso 4: Integrar en FlightHUD

**En `core/FlightHUD.h`**:
```cpp
#include "../instruments/flight/AttitudeIndicator.h"

private:
    AttitudeIndicator* attitudeIndicator_;  // Referencia rápida
```

**En `core/FlightHUD.cpp` (constructor)**:
```cpp
// Crear instrumento
auto attitudeIndicator = std::make_unique<AttitudeIndicator>();
attitudeIndicator_ = attitudeIndicator.get();
instruments_.push_back(std::move(attitudeIndicator));
```

**En `core/FlightHUD.cpp` (setupInstrumentLayout)**:
```cpp
// Configurar posición y tamaño
{
    const float SIZE = 300.0f;
    float posX = centerX - SIZE * 0.5f;
    float posY = centerY - SIZE * 0.5f;
    
    attitudeIndicator_->setPosition(glm::vec2(posX, posY));
    attitudeIndicator_->setSize(glm::vec2(SIZE, SIZE));
    attitudeIndicator_->setColor(hudColor_);
}
```

### Paso 5: Actualizar Makefile

Ya está configurado para buscar en subdirectorios, pero verifica:
```makefile
CPP_SOURCES = ... $(wildcard src/hud/instruments/flight/*.cpp) ...
```

### Paso 6: Compilar y Probar

```bash
make clean
make -j4
./build/FlightSim-HUD
```

---

## Convenciones de Código

### Nombres de Archivos
- PascalCase para clases: `AttitudeIndicator.h`
- Coincidencia exacta entre nombre de clase y archivo

### Estructura de Includes
```cpp
// Order de includes:
#include "Header.h"              // Header propio primero
#include "../../ruta/relativa"   // Headers del proyecto
#include <library>               // Bibliotecas estándar
```

### Namespace
Todos los instrumentos en `namespace hud`:
```cpp
namespace hud {
    class MyInstrument : public Instrument {
        // ...
    };
}
```

### Constantes
Definir constantes de configuración al inicio:
```cpp
namespace hud {
    // Configuración visual
    static const float INDICATOR_WIDTH = 120.0f;
    static const float INDICATOR_HEIGHT = 450.0f;
    static const float SCALE_SPACING = 30.0f;
    
    class MyInstrument : public Instrument {
        // ...
    };
}
```

---

## Rutas de Includes Relativas

Desde cada ubicación, las rutas son:

### Desde `core/`:
```cpp
#include "../../gfx/rendering/Renderer2D.h"
#include "../../flight/data/FlightData.h"
```

### Desde `instruments/flight/`:
```cpp
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/TextRenderer.h"
```

### Desde `instruments/navigation/`:
```cpp
#include "../../core/Instrument.h"
#include "../../../gfx/rendering/TextRenderer.h"
```

---

## Dependencias del Módulo

### Internas (dentro de src/)
- **gfx/rendering/** - Sistema de renderizado 2D
  - `Renderer2D` - Primitivas gráficas
  - `TextRenderer` - Renderizado de texto
- **flight/** - Datos de vuelo
  - `FlightData` - Estructura con telemetría

### Externas
- **GLM** - Matemáticas vectoriales
- **OpenGL** - Renderizado

---

## Testing y Validación

### Checklist para Nuevos Instrumentos

- [ ] Hereda correctamente de `Instrument`
- [ ] Implementa `render()` override
- [ ] Usa `position_`, `size_`, `color_` heredados
- [ ] Respeta `enabled_` flag
- [ ] Incluye documentación en header
- [ ] Sigue convenciones de naming
- [ ] Compila sin warnings
- [ ] Se integra en FlightHUD correctamente
- [ ] No causa conflictos visuales con otros instrumentos

### Verificación Visual

1. Ejecutar simulador en vista POV
2. Verificar posicionamiento correcto
3. Confirmar legibilidad de texto
4. Validar escala y proporciones
5. Probar en diferentes resoluciones

---

## Instrumentos Futuros Planificados

### Alta Prioridad
- [ ] **AttitudeIndicator** - Horizonte artificial (flight/)
- [ ] **HeadingIndicator** - Indicador de rumbo (navigation/)
- [ ] **VerticalSpeedIndicator** - VSI (flight/)

### Media Prioridad
- [ ] **TurnCoordinator** - Coordinador de giro (flight/)
- [ ] **CompassRose** - Rosa de los vientos (navigation/)
- [ ] **RadioAltimeter** - Altímetro de radio (flight/)

### Baja Prioridad
- [ ] **FuelGauge** - Indicador de combustible (engine/)
- [ ] **EngineRPM** - RPM del motor (engine/)
- [ ] **WarningPanel** - Panel de advertencias (systems/)

---

## Recursos y Referencias

### Documentación Relacionada
- **WAYPOINT_INDICATOR_README.md** - Manual completo del indicador de waypoints
- **WAYPOINT_NAVIGATION.md** - Sistema de navegación
- **docs/README_INSTRUMENTS.md** - Guía general de instrumentos

### Aviación Real
- FAA Instrument Flying Handbook
- ICAO Annex 6 (Aircraft Instruments)
- EASA CS-25 (Cockpit Standards)

### Tutoriales
- OpenGL 2D Rendering
- HUD Design Best Practices
- Game UI/UX Patterns

---

## Historial de Cambios

### v2.0 - Reorganización del Módulo (2025)
- ✅ Estructura por categorías (core/, instruments/)
- ✅ Separación flight/ y navigation/
- ✅ Documentación actualizada
- ✅ Compilación verificada

### v1.5 - WaypointIndicator Profesional (2025)
- ✅ Rediseño completo del indicador
- ✅ Panel dual HSI militar
- ✅ Métricas digitales

### v1.0 - Sistema Base (2025)
- ✅ Clase Instrument abstracta
- ✅ FlightHUD coordinador
- ✅ Altimeter implementado
- ✅ SpeedIndicator base

---

## Contacto y Soporte

Para preguntas sobre el módulo HUD:
1. Consultar esta documentación
2. Revisar código de ejemplo (Altimeter, WaypointIndicator)
3. Verificar patrones de diseño en FlightHUD

---

## Licencia

Parte del proyecto FlightSim-HUD.
Ver LICENSE en la raíz del proyecto.
