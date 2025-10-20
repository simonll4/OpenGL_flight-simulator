# Flight Simulator HUD

[![Status](https://img.shields.io/badge/status-active-success.svg)]()
[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()

Sistema de HUD (Heads-Up Display) profesional para simulador de vuelo con modelo F-16.

![OpenGL](https://img.shields.io/badge/OpenGL-3.3-blue)
![C++](https://img.shields.io/badge/C++-17-blue)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)

---

## Descripción

Simulador de vuelo 3D completo con sistema HUD profesional que incluye:

- ✈️ **Modelo F-16** con texturizado completo
- 🗺️ **Terreno realista** con textura triplanar y niebla
- 🌅 **Skybox** envolvente para cielo realista
- 📊 **Sistema HUD modular** con múltiples instrumentos
- 🧭 **Navegación por waypoints** estilo militar profesional
- 🎮 **Controles de vuelo** completos (pitch, roll, yaw)

---

## Características Principales

### Sistema HUD (Heads-Up Display)
- **Altímetro digital** con tape vertical
- **Indicador de velocidad** (próximamente)
- **Navegación por waypoints** con panel HSI profesional
- **Arquitectura modular** para agregar instrumentos fácilmente

### Gráficos 3D
- Renderizado OpenGL 3.3+
- Modelo F-16 cargado con Assimp
- Terreno procedural con texturizado triplanar
- Skybox con atlas de texturas
- Sistema de shaders modular

### Navegación
- Sistema de waypoints con desaparición al capturar
- Indicador HSI (Horizontal Situation Indicator) profesional
- Detección automática de misión completada
- Waypoints 3D renderizados en el mundo

---

## Capturas de Pantalla

```
[Vista POV con HUD completo]
- Altímetro (derecha)
- Waypoint Indicator (centro superior)
- Modelo F-16 con texturizado
- Terreno y skybox
```

---

## Requisitos

### Sistema
- **SO**: Linux (Ubuntu 20.04+ recomendado)
- **GPU**: Soporte OpenGL 3.3+
- **RAM**: 2GB mínimo, 4GB recomendado

### Bibliotecas

```bash
# Ubuntu/Debian
sudo apt install build-essential
sudo apt install libglfw3-dev
sudo apt install libglm-dev
sudo apt install libassimp-dev
```

#### Bibliotecas Incluidas
- **GLAD** - OpenGL function loader (incluido)
- **GLM** - Matemáticas vectoriales
- **GLFW** - Gestión de ventanas y entrada
- **Assimp** - Carga de modelos 3D
- **stb_image** - Carga de texturas (incluido)

---

## Compilación

### Quick Start

```bash
# Clonar repositorio
git clone [repo-url]
cd HUD

# Compilar
make clean
make -j4

# Ejecutar
./build/FlightSim-HUD
```

### Makefile Targets

```bash
make          # Compilar proyecto
make clean    # Limpiar archivos compilados
make -j4      # Compilar con 4 threads
```

---

## Estructura del Proyecto

```
HUD/
├── src/                    # Código fuente
│   ├── main.cpp           # Punto de entrada
│   ├── hud/               # Módulo HUD
│   │   ├── core/          # Sistema base
│   │   └── instruments/   # Instrumentos organizados
│   ├── gfx/               # Sistema gráfico
│   │   ├── core/          # Shaders y base
│   │   ├── geometry/      # Modelos y meshes
│   │   ├── rendering/     # Renderizado 2D/3D
│   │   ├── terrain/       # Terreno procedural
│   │   └── skybox/        # Sistema de cielo
│   └── flight/            # Datos de vuelo
│
├── shaders/               # Shaders GLSL
├── assets/                # Recursos (modelos, texturas)
├── docs/                  # Documentación completa
│   ├── hud/              # Docs del módulo HUD
│   ├── gfx/              # Docs del sistema gráfico
│   └── waypoints/        # Docs de navegación
├── build/                 # Ejecutable compilado
└── include/              # Headers externos

```

---

## Controles

### Vuelo

| Tecla | Acción |
|-------|--------|
| **W** | Pitch Up (cabeceo arriba) |
| **S** | Pitch Down (cabeceo abajo) |
| **A** | Yaw Left (guiñada izquierda) |
| **D** | Yaw Right (guiñada derecha) |
| **Q** | Roll Left (alabeo izquierda) |
| **E** | Roll Right (alabeo derecha) |
| **↑** | Aumentar velocidad |
| **↓** | Disminuir velocidad |

### Cámara y Sistema

| Tecla | Acción |
|-------|--------|
| **V** | Cambiar vista (POV ↔ 3ra persona) |
| **N** | Activar/Desactivar waypoints |
| **M** | Saltar waypoint actual |
| **R** | Reiniciar misión |
| **ESC** | Salir |

---

## Documentación

Toda la documentación está centralizada en `/docs`:

### Documentación del HUD
📖 **[Arquitectura del Módulo](docs/hud/README_MODULE.md)**  
📖 **[Guía de Instrumentos](docs/hud/README_INSTRUMENTS.md)**  
📖 **[Guía de Migración](docs/hud/MIGRATION_GUIDE.md)**  

### Sistema de Navegación
📖 **[Manual del Waypoint Indicator](docs/waypoints/WAYPOINT_INDICATOR_README.md)**  
📖 **[Sistema de Navegación Completo](docs/waypoints/WAYPOINT_NAVIGATION.md)**  

### Sistema Gráfico
📖 **[Módulo GFX](docs/gfx/README.md)**  

### Desarrollo
📖 **[Guía de Estilo de Código](docs/CODING_STYLE.md)**  
📖 **[Índice Completo de Documentación](docs/INDEX.md)**  

---

## Arquitectura

### Módulos Principales

#### HUD (Heads-Up Display)
- **Ubicación**: `src/hud/`
- **Función**: Sistema de interfaz de usuario en tiempo real
- **Arquitectura**: Modular, basada en herencia polimórfica
- **Instrumentos**: Altímetro, SpeedIndicator, WaypointIndicator

#### GFX (Graphics)
- **Ubicación**: `src/gfx/`
- **Función**: Sistema de renderizado 2D y 3D
- **Componentes**: Shaders, Modelos, Terreno, Skybox, Renderers

#### Flight Data
- **Ubicación**: `src/flight/`
- **Función**: Telemetría y datos de vuelo
- **Datos**: Actitud, navegación, velocidad, posición

---

## Desarrollo

### Agregar un Nuevo Instrumento HUD

1. **Decidir categoría** (flight, navigation, engine, etc.)
2. **Crear archivos** en `src/hud/instruments/[categoria]/`
3. **Heredar de** `Instrument`
4. **Implementar** `render()` override
5. **Integrar en** `FlightHUD`

Guía completa: [docs/hud/README_MODULE.md](docs/hud/README_MODULE.md)

### Coding Standards

- **C++ Standard**: C++17
- **Estilo**: CamelCase para clases, snake_case para variables
- **Includes**: Rutas relativas desde src/
- **Namespaces**: Por módulo (hud, gfx, flight)
- **Documentación**: Doxygen-style comments

---

## Sistema de Waypoints

El simulador incluye un sistema completo de navegación por waypoints:

### Características
- **Indicador HSI** profesional estilo militar
- **Panel dual**: Brújula circular + información digital
- **Métricas**: Distancia (DIST), Ángulo de giro (TURN), Rumbo (BRG)
- **Barra de proximidad** visual
- **Desaparición automática** al capturar waypoints
- **Detección de misión completada**

### Circuito por Defecto
- 6 waypoints formando un circuito hexagonal
- Distancias: 1500-2000 metros
- Altitudes variables: 100-200 metros
- Radio de captura: 80 metros

Documentación completa: [docs/waypoints/](docs/waypoints/)

---

## Rendimiento

### Especificaciones
- **FPS objetivo**: 60 FPS
- **Resolución**: 1280x720 (configurable)
- **Vértices terreno**: 16,641
- **Triángulos terreno**: 98,304
- **Modelo F-16**: 9 meshes, 5 materiales

### Optimizaciones
- Culling de objetos fuera de vista
- Mipmapping en texturas
- Instanciado para waypoints
- Batch rendering en HUD

---

## Resolución de Problemas

### El HUD no se muestra
**Solución**: Presiona **V** para cambiar a vista POV (primera persona)

### Errores de compilación
**Solución**: 
```bash
make clean
make -j4
```

### Waypoints no visibles
**Solución**: Presiona **N** para activar el sistema de waypoints

### Performance bajo
**Solución**: Reducir resolución o densidad del terreno en código

---

## Roadmap

### Versión Actual: v2.0
- ✅ Sistema HUD modular completo
- ✅ Waypoint Indicator profesional
- ✅ Documentación centralizada
- ✅ Arquitectura organizada

### Próximas Versiones

#### v2.1
- [ ] Attitude Indicator (horizonte artificial)
- [ ] Heading Indicator (rumbo)
- [ ] Activar SpeedIndicator

#### v2.2
- [ ] VSI (Vertical Speed Indicator)
- [ ] Turn Coordinator
- [ ] Radio Altimeter

#### v3.0
- [ ] Sistema de motor (RPM, fuel, temp)
- [ ] Panel de advertencias
- [ ] Sistema de misiones avanzado

---

## Contribuir

### Cómo Contribuir

1. Fork el proyecto
2. Crear rama feature (`git checkout -b feature/AmazingFeature`)
3. Commit cambios (`git commit -m 'Add AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abrir Pull Request

### Guidelines
- Seguir coding standards del proyecto
- Agregar documentación para nuevas características
- Incluir comentarios en código complejo
- Probar antes de hacer PR

---

## Créditos

### Tecnologías Utilizadas
- **OpenGL** - API gráfica
- **GLFW** - Window management
- **GLM** - Matemáticas
- **Assimp** - Carga de modelos
- **stb_image** - Carga de texturas

### Recursos
- Modelo F-16: [Fuente]
- Texturas terreno: Forrest Ground 4K
- Skybox: Custom atlas

### Inspiración
- Sistemas HSI reales de aviación
- HUD de combate militar
- Simuladores profesionales (DCS, MSFS)

---

## Licencia

Este proyecto está bajo licencia MIT. Ver `LICENSE` para más detalles.

---

## Contacto

**Proyecto**: Flight Simulator HUD  
**Versión**: 2.0  
**Última actualización**: 2025  

---

## Agradecimientos

- Comunidad OpenGL por recursos y tutoriales
- LearnOpenGL.com por guías de referencia
- Comunidad de aviación por especificaciones técnicas
- Contribuidores del proyecto

---

**¡Feliz vuelo!** ✈️
