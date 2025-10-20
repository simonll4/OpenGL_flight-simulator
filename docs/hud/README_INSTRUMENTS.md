# Guía para Agregar Nuevos Instrumentos al HUD

Esta guía explica cómo agregar nuevos instrumentos al sistema de HUD utilizando la arquitectura de clase base `Instrument`.

## Arquitectura

El sistema de HUD utiliza **herencia polimórfica** para gestionar instrumentos de forma uniforme:

```
Instrument (clase base abstracta)
├── Altimeter
├── AttitudeIndicator (TODO)
├── AirspeedIndicator (TODO)
├── HeadingIndicator (TODO)
└── VerticalSpeedIndicator (TODO)
```

### Ventajas de esta arquitectura:

✅ **Reutilización de código**: Propiedades comunes (posición, tamaño, color) ya están implementadas
✅ **Gestión polimórfica**: Todos los instrumentos se renderizan automáticamente en un loop
✅ **Extensibilidad**: Fácil agregar nuevos instrumentos sin modificar el código existente
✅ **Mantenibilidad**: Cada instrumento tiene su propia lógica encapsulada

## Paso a Paso: Crear un Nuevo Instrumento

### 1. Crear los archivos del instrumento

Crea dos archivos en `src/hud/`:
- `NombreInstrumento.h`
- `NombreInstrumento.cpp`

### 2. Definir la clase heredando de `Instrument`

**Ejemplo: `AttitudeIndicator.h`**

```cpp
#pragma once
#include "Instrument.h"

namespace hud
{
    /**
     * @class AttitudeIndicator
     * @brief Horizonte artificial - muestra pitch y roll
     */
    class AttitudeIndicator : public Instrument
    {
    public:
        AttitudeIndicator();

        /**
         * @brief Renderiza el horizonte artificial
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Métodos específicos de este instrumento
        void drawSkyGround(gfx::Renderer2D &renderer, float pitch, float roll);
        void drawPitchLadder(gfx::Renderer2D &renderer, float pitch, float roll);
        void drawRollIndicator(gfx::Renderer2D &renderer, float roll);
        void drawAircraftSymbol(gfx::Renderer2D &renderer);
    };

} // namespace hud
```

### 3. Implementar el constructor y el método `render()`

**Ejemplo: `AttitudeIndicator.cpp`**

```cpp
#include "AttitudeIndicator.h"

namespace hud
{
    AttitudeIndicator::AttitudeIndicator() : Instrument()
    {
        // Configuración específica de este instrumento
        size_ = glm::vec2(300.0f, 300.0f);  // Cuadrado
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f);
    }

    void AttitudeIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        // Extraer datos relevantes
        float pitch = flightData.pitch;
        float roll = flightData.roll;

        // Dibujar elementos del horizonte artificial
        drawSkyGround(renderer, pitch, roll);
        drawPitchLadder(renderer, pitch, roll);
        drawRollIndicator(renderer, roll);
        drawAircraftSymbol(renderer);
    }

    // Implementar métodos privados...
    void AttitudeIndicator::drawSkyGround(...) { /* ... */ }
    void AttitudeIndicator::drawPitchLadder(...) { /* ... */ }
    // etc.

} // namespace hud
```

### 4. Agregar el instrumento a `FlightHUD`

**a) Incluir el header en `FlightHUD.h`:**

```cpp
#include "AttitudeIndicator.h"
```

**b) Agregar referencia rápida (opcional) en `FlightHUD.h`:**

```cpp
private:
    // Referencias rápidas a instrumentos específicos
    Altimeter* altimeter_;
    AttitudeIndicator* attitudeIndicator_;  // ← Nueva línea
```

**c) Crear el instrumento en el constructor de `FlightHUD.cpp`:**

```cpp
FlightHUD::FlightHUD() : altimeter_(nullptr), attitudeIndicator_(nullptr), ...
{
    // ... código existente ...
    
    // Altimeter
    auto altimeter = std::make_unique<Altimeter>();
    altimeter_ = altimeter.get();
    instruments_.push_back(std::move(altimeter));

    // AttitudeIndicator ← Nuevo
    auto attitudeIndicator = std::make_unique<AttitudeIndicator>();
    attitudeIndicator_ = attitudeIndicator.get();
    instruments_.push_back(std::move(attitudeIndicator));
}
```

**d) Configurar layout en `setupInstrumentLayout()`:**

```cpp
void FlightHUD::setupInstrumentLayout()
{
    float centerX = screenWidth_ * 0.5f;
    float centerY = screenHeight_ * 0.5f;

    // ... código del altímetro ...

    // ATTITUDE INDICATOR (Horizonte Artificial) - CENTRO
    {
        const float SIZE = 300.0f;  // Cuadrado
        float posX = centerX - SIZE * 0.5f;
        float posY = centerY - SIZE * 0.5f;

        attitudeIndicator_->setPosition(glm::vec2(posX, posY));
        attitudeIndicator_->setSize(glm::vec2(SIZE, SIZE));
        attitudeIndicator_->setColor(hudColor_);
    }
}
```

### 5. Compilar

El Makefile detectará automáticamente el nuevo archivo `.cpp`:

```bash
make clean
make
```

## Propiedades Heredadas de `Instrument`

Todos los instrumentos tienen acceso automático a:

| Propiedad | Tipo | Descripción |
|-----------|------|-------------|
| `position_` | `glm::vec2` | Posición en pantalla (x, y) |
| `size_` | `glm::vec2` | Tamaño (ancho, alto) |
| `color_` | `glm::vec4` | Color RGBA principal |
| `enabled_` | `bool` | Si está visible/activo |

## Renderizado Automático

Una vez agregado al vector `instruments_`, el nuevo instrumento **se renderiza automáticamente**:

```cpp
void FlightHUD::render()
{
    // ...
    for (const auto& instrument : instruments_)
    {
        if (instrument && instrument->isEnabled())
        {
            instrument->render(*renderer2D_, currentFlightData_);  // ← Polimorfismo
        }
    }
    // ...
}
```

## Checklist para Nuevo Instrumento

- [ ] Crear `NombreInstrumento.h` y `.cpp`
- [ ] Heredar de `Instrument`
- [ ] Implementar constructor con configuración inicial
- [ ] Implementar `render()` override
- [ ] Incluir header en `FlightHUD.h`
- [ ] Agregar referencia en `FlightHUD.h` (opcional)
- [ ] Crear instancia en constructor de `FlightHUD`
- [ ] Configurar posición/tamaño en `setupInstrumentLayout()`
- [ ] Compilar y probar

## Instrumentos Pendientes (TODO)

1. **AttitudeIndicator** - Horizonte artificial (pitch y roll)
2. **AirspeedIndicator** - Velocidad del aire (tape vertical izquierdo)
3. **HeadingIndicator** - Brújula/rumbo (tape horizontal superior)
4. **VerticalSpeedIndicator** - Variometro (tape vertical derecho)
5. **TurnCoordinator** - Coordinador de viraje
6. **CompassRose** - Rosa de los vientos

## Notas Adicionales

- **Método virtual puro**: `render()` DEBE ser implementado en cada clase derivada
- **Polimorfismo**: No necesitas modificar el código de renderizado en `FlightHUD`
- **Referencias raw**: Las referencias `altimeter_`, etc. son solo para acceso directo en configuración
- **Ownership**: Los `unique_ptr` en `instruments_` gestionan la memoria automáticamente
