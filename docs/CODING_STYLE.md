# Guía de Estilo de Código - Flight Simulator HUD

Este documento establece las convenciones de código para mantener consistencia y legibilidad en el proyecto.

---

## Principios Generales

1. **Claridad sobre concisión** - El código debe ser autoexplicativo
2. **Consistencia** - Seguir el estilo existente en el módulo
3. **Documentación** - Comentarios para lógica compleja
4. **Performance** - Optimizar sin sacrificar legibilidad

---

## Nomenclatura

### Archivos

```cpp
// Formato: PascalCase
Altimeter.h
Altimeter.cpp
WaypointIndicator.h
FlightHUD.cpp
```

**Regla**: Nombre de archivo == Nombre de clase principal

### Clases y Estructuras

```cpp
// PascalCase
class Instrument { };
class WaypointIndicator { };
struct FlightData { };
struct Waypoint { };
```

### Métodos y Funciones

```cpp
// camelCase
void render();
void updateFromCamera();
float calculateBearing();
bool isEnabled();
```

### Variables

```cpp
// snake_case con sufijo para miembros de clase
class Example {
private:
    float speed_;           // Miembro privado
    glm::vec2 position_;   // Miembro privado
    bool enabled_;         // Miembro privado
};

// camelCase para variables locales
void function() {
    float localSpeed = 0.0f;
    int waypointCount = 5;
    bool isActive = true;
}
```

### Constantes

```cpp
// UPPER_SNAKE_CASE para constantes globales
static const float MAX_SPEED = 300.0f;
static const int BUFFER_SIZE = 1024;

// UPPER_SNAKE_CASE para constantes de configuración
static const float CIRCLE_RADIUS = 60.0f;
static const float ARROW_LENGTH = 40.0f;
```

### Namespaces

```cpp
// Minúsculas, por módulo
namespace hud { }
namespace gfx { }
namespace flight { }
```

---

## Estructura de Archivos

### Headers (.h)

```cpp
#pragma once                    // Siempre usar pragma once
#include <system_headers>       // Headers del sistema primero
#include "local_headers.h"      // Headers del proyecto después

namespace module_name          // Namespace del módulo
{
    /**
     * @class ClassName
     * @brief Descripción breve
     *
     * Descripción detallada con:
     * - Propósito de la clase
     * - Responsabilidades
     * - Ejemplo de uso si es complejo
     */
    class ClassName
    {
    public:
        // Constructor/Destructor
        ClassName();
        virtual ~ClassName() = default;

        // Métodos públicos agrupados por funcionalidad
        void publicMethod();

        // Getters
        int getValue() const;

        // Setters
        void setValue(int value);

    protected:
        // Métodos protegidos

    private:
        // Métodos privados
        void privateHelper();

        // Miembros privados con comentarios inline
        int value_;    ///< Descripción breve del miembro
        bool flag_;    ///< Estado del flag
    };

} // namespace module_name
```

### Implementation (.cpp)

```cpp
#include "ClassName.h"          // Header propio primero
#include "other_headers.h"      // Otros headers del proyecto
#include <system_headers>       // System headers al final

namespace module_name
{
    // ===========================================================================
    // SECCIÓN BIEN DEFINIDA
    // ===========================================================================

    ClassName::ClassName()
        : value_(0)
        , flag_(false)
    {
        // Inicialización en constructor
    }

    void ClassName::publicMethod()
    {
        // Implementación
        // Comentarios para lógica compleja
    }

} // namespace module_name
```

---

## Comentarios y Documentación

### Comentarios de Clase (Doxygen Style)

```cpp
/**
 * @class Altimeter
 * @brief Instrumento que muestra la altitud del avión
 *
 * Hereda de Instrument y proporciona:
 * - Tape vertical con escala de altitud móvil
 * - Caja de lectura digital con display de 7 segmentos
 * - Auto-escala según rango de altitud
 *
 * Ejemplo de uso:
 * @code
 * Altimeter altimeter;
 * altimeter.setPosition(glm::vec2(100, 100));
 * altimeter.render(renderer, flightData);
 * @endcode
 */
class Altimeter : public Instrument { };
```

### Comentarios de Métodos

```cpp
/**
 * @brief Renderiza el indicador de waypoint
 * @param renderer Renderer 2D compartido para primitivas
 * @param flightData Datos actuales del vuelo con telemetría
 *
 * Este método renderiza:
 * - Brújula circular con marcas cardinales
 * - Flecha direccional apuntando al waypoint
 * - Información digital (distancia, ángulo, bearing)
 * - Barra de proximidad visual
 */
void render(gfx::Renderer2D& renderer, const flight::FlightData& flightData) override;
```

### Comentarios Inline

```cpp
// Comentario corto para líneas específicas
float speed = 100.0f; // Velocidad inicial en kt

// Bloque de comentario para secciones
// Este bloque calcula el bearing al waypoint proyectando
// el vector en el plano horizontal (XZ) y usando atan2
float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y);
```

### Secciones de Código

```cpp
// ============================================================================
// SECCIÓN PRINCIPAL EN MAYÚSCULAS
// ============================================================================

// ----------------------------------------------------------------------------
// Subsección con guiones
// ----------------------------------------------------------------------------
```

### Comentarios TODO

```cpp
// TODO: Implementar cálculo de ETA
// FIXME: Bug con valores negativos de bearing
// NOTE: Este método asume coordenadas en metros
// HACK: Workaround temporal hasta refactorización
```

---

## Formato y Estilo

### Indentación

```cpp
// 4 espacios (NO tabs)
class Example {
public:
    void method()
    {
        if (condition)
        {
            doSomething();
        }
    }
};
```

### Llaves

```cpp
// Estilo Allman (llaves en nueva línea)
void function()
{
    if (condition)
    {
        // código
    }
    else
    {
        // código
    }
}

// Excepción: Lambdas pueden usar estilo compacto
auto lambda = [](int x) { return x * 2; };
```

### Espaciado

```cpp
// Espacios alrededor de operadores
int result = a + b * c;
bool flag = (x == y) && (z != w);

// Sin espacio entre función y paréntesis
void function(int param);

// Espacio después de palabra clave
if (condition)
for (int i = 0; i < count; ++i)
while (running)
```

### Longitud de Línea

```cpp
// Máximo 100 caracteres por línea
// Si se excede, partir en múltiples líneas

// Bien:
renderer.drawLine(start, end, color, thickness);

// Partir si es muy largo:
renderer.drawLine(
    startPosition,
    endPosition,
    glm::vec4(0.0f, 1.0f, 0.4f, 0.95f),
    thickness);
```

---

## Includes

### Orden de Includes

```cpp
#include "Header.h"              // 1. Header propio

#include "../../path/relative"   // 2. Headers del proyecto (relativos)
#include "module/header.h"

#include <glm/glm.hpp>          // 3. Bibliotecas externas
#include <vector>
#include <string>               // 4. STL (alfabético)
```

### Guards de Includes

```cpp
// Siempre usar #pragma once
#pragma once

// NO usar guards tradicionales
```

---

## Tipos y Const-correctness

### Referencias y Punteros

```cpp
// Referencias constantes para parámetros
void process(const std::string& text);
void render(const flight::FlightData& data);

// Punteros para opcionalidad o ownership
void setParent(Instrument* parent);  // Puede ser nullptr
std::unique_ptr<Instrument> create();  // Transfer ownership
```

### Const-correctness

```cpp
class Example
{
public:
    // Métodos const para no modificar estado
    int getValue() const { return value_; }
    bool isReady() const;

    // Métodos no-const para modificar estado
    void setValue(int v) { value_ = v; }

private:
    int value_;
};

// Variables const cuando sea posible
const float RADIUS = 45.0f;
const glm::vec2 position = getPosition();
```

---

## Patterns y Mejores Prácticas

### RAII (Resource Acquisition Is Initialization)

```cpp
// Usar constructores/destructores para recursos
class ResourceHolder
{
public:
    ResourceHolder() { /* acquire */ }
    ~ResourceHolder() { /* release */ }

    // Prevenir copia accidental
    ResourceHolder(const ResourceHolder&) = delete;
    ResourceHolder& operator=(const ResourceHolder&) = delete;
};
```

### Smart Pointers

```cpp
// Preferir smart pointers sobre raw pointers
std::unique_ptr<Instrument> instrument;  // Ownership único
std::shared_ptr<Texture> texture;        // Ownership compartido

// Raw pointers solo para no-owning references
Instrument* getInstrument();  // No transfiere ownership
```

### Inicialización

```cpp
// Preferir lista de inicialización
ClassName::ClassName(int value, const std::string& name)
    : value_(value)
    , name_(name)
    , initialized_(true)
{
    // Cuerpo del constructor vacío o mínimo
}

// Uniform initialization
glm::vec2 position{100.0f, 200.0f};
std::vector<int> numbers{1, 2, 3, 4, 5};
```

### Enums

```cpp
// Usar enum class para type safety
enum class State
{
    Idle,
    Running,
    Paused,
    Stopped
};

// Uso:
State currentState = State::Running;
```

---

## Organización de Código

### Por Responsabilidad

```cpp
// Agrupar métodos relacionados
class Instrument
{
public:
    // Constructores
    Instrument();
    virtual ~Instrument();

    // Configuración
    void setPosition(const glm::vec2& pos);
    void setSize(const glm::vec2& size);
    void setColor(const glm::vec4& color);

    // Getters
    const glm::vec2& getPosition() const;
    const glm::vec2& getSize() const;

    // Renderizado
    virtual void render(...) = 0;
};
```

### Por Visibilidad

```cpp
class Example
{
public:     // Interfaz pública primero
    // ...

protected:  // Interfaz protegida
    // ...

private:    // Implementación privada al final
    // ...
};
```

---

## Performance

### Evitar Copias Innecesarias

```cpp
// Mal: Copia vector completo
void process(std::vector<int> data);

// Bien: Referencia constante
void process(const std::vector<int>& data);

// Si necesitas modificar:
void process(std::vector<int>& data);
```

### Reservar Memoria

```cpp
// Reservar capacidad si se conoce el tamaño
std::vector<Waypoint> waypoints;
waypoints.reserve(10);  // Evita reallocaciones
```

### Operaciones en Loops

```cpp
// Mal: Recalcular cada iteración
for (int i = 0; i < vector.size(); ++i)

// Bien: Calcular una vez
const size_t size = vector.size();
for (size_t i = 0; i < size; ++i)

// Mejor: Range-based for
for (const auto& item : vector)
```

---

## Error Handling

### Asserts

```cpp
#include <cassert>

void process(Instrument* instrument)
{
    assert(instrument != nullptr);
    // ...
}
```

### Return Early

```cpp
// Preferir early returns para simplificar lógica
void render()
{
    if (!enabled_)
        return;  // Early return

    if (!hasData())
        return;

    // Lógica principal sin nesting profundo
    drawBackground();
    drawContent();
}
```

---

## Ejemplos Completos

### Ejemplo de Header Completo

```cpp
#pragma once
#include "../../core/Instrument.h"
#include <string>

namespace hud
{
    /**
     * @class ExampleInstrument
     * @brief Ejemplo de instrumento bien documentado
     *
     * Este instrumento muestra cómo estructurar correctamente
     * un header siguiendo las convenciones del proyecto.
     */
    class ExampleInstrument : public Instrument
    {
    public:
        // ===================================================================
        // CONSTRUCCIÓN Y DESTRUCCIÓN
        // ===================================================================

        /**
         * @brief Constructor con parámetros por defecto
         * @param name Nombre del instrumento
         */
        explicit ExampleInstrument(const std::string& name = "Example");

        /**
         * @brief Destructor virtual
         */
        virtual ~ExampleInstrument() = default;

        // No copiable
        ExampleInstrument(const ExampleInstrument&) = delete;
        ExampleInstrument& operator=(const ExampleInstrument&) = delete;

        // ===================================================================
        // INTERFAZ PÚBLICA
        // ===================================================================

        /**
         * @brief Renderiza el instrumento
         * @param renderer Renderer 2D
         * @param flightData Datos de vuelo
         */
        void render(gfx::Renderer2D& renderer, 
                   const flight::FlightData& flightData) override;

        /**
         * @brief Actualiza estado interno
         * @param deltaTime Tiempo transcurrido en segundos
         */
        void update(float deltaTime);

        // ===================================================================
        // GETTERS/SETTERS
        // ===================================================================

        const std::string& getName() const { return name_; }
        void setName(const std::string& name) { name_ = name; }

    private:
        // ===================================================================
        // MÉTODOS PRIVADOS AUXILIARES
        // ===================================================================

        void drawBackground(gfx::Renderer2D& renderer);
        void drawContent(gfx::Renderer2D& renderer);

        // ===================================================================
        // MIEMBROS PRIVADOS
        // ===================================================================

        std::string name_;    ///< Nombre del instrumento
        float value_;         ///< Valor actual mostrado
        bool needsUpdate_;    ///< Flag de actualización pendiente
    };

} // namespace hud
```

---

## Checklist de Code Review

Antes de hacer commit, verificar:

- [ ] Código sigue convenciones de nomenclatura
- [ ] Headers tienen documentación Doxygen
- [ ] Métodos complejos tienen comentarios
- [ ] Indentación consistente (4 espacios)
- [ ] Sin warnings de compilación
- [ ] Includes organizados correctamente
- [ ] Const-correctness aplicado
- [ ] Sin copias innecesarias
- [ ] Early returns donde sea apropiado
- [ ] TODO/FIXME documentados si aplica

---

## Herramientas

### Formateo Automático

```bash
# clang-format (si disponible)
clang-format -i *.cpp *.h

# Configuración en .clang-format
BasedOnStyle: LLVM
IndentWidth: 4
```

### Análisis Estático

```bash
# cppcheck
cppcheck --enable=all src/

# Compilar con warnings
g++ -Wall -Wextra -Wpedantic ...
```

---

## Referencias

- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- [Effective C++](https://www.aristeia.com/books.html) - Scott Meyers

---

**Mantener el código limpio es responsabilidad de todos.** ✨
