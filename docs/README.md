# Flight Simulator HUD – Documentación

Este directorio resume los aspectos esenciales del simulador para que nuevas personas en el equipo puedan orientarse rápido sin leer reportes de limpieza ni bitácoras antiguas.

## 1. Propósito del Proyecto
- **Tipo**: simulador de vuelo en C++/OpenGL 3.3 enfocado en el HUD del F-16.
- **Objetivo**: validar instrumentación, navegación y flujo de misiones sobre un entorno 3D ligero.
- **Componentes principales**:
  - `src/core` – ciclo de vida de la aplicación y administración de estados.
  - `src/systems` – simulador de vuelo (DLFDM), cámara y waypoints.
  - `src/ui` – menús, planificador, overlay y HUD basado en `Renderer2D`.
  - `src/gfx` – recursos 3D (terreno plano texturizado, skybox, modelo y shaders).

## 2. Cómo Compilar y Ejecutar
1. Instalar dependencias (Ubuntu/Debian):
   ```bash
   sudo apt install build-essential libglfw3-dev libglm-dev libassimp-dev
   ```
2. Compilar:
   ```bash
   make -j4
   ```
3. Ejecutar:
   ```bash
   ./build/FlightSim-HUD
   ```

## 3. Flujo de Estados en Tiempo de Ejecución
```
MenuState (selección de misión)
   ↓
PlanningState (editor y briefing)
   ↓
FlightState (simulación + HUD + overlay)
```
- **MenuState**: usa `UIManager::MissionMenu` + `MissionRegistry`.
- **PlanningState**: expone el planificador gráfico por waypoints.
- **FlightState**: orquesta simulación, cámara, HUD, waypoints y overlay.

## 4. Subsistemas Clave
- **Simulación de Vuelo** (`systems/FlightSimulationController`)
  - Wrapper de `flight/dlfdm/FdmSimulation`.
  - Devuelve `flight::FlightData`, fuente oficial para instrumentos/UI.
- **Gráficos** (`gfx/terrain/TerrainPlane`, `gfx/skybox`, `gfx/geometry`)
  - Terreno plano texturizado (grid 3x3 siguiendo a la cámara con tiling), skybox + cubemap, modelo del avión con shader PBR básico.
- **HUD** (`hud/core/FlightHUD`)
  - Instrumentos modulares (ver `docs/hud.md`).
  - Se renderiza sobre la vista cuando la cámara está en primera persona.
- **Sistema de Misiones** (`mission/**`)
  - `MissionRegistry` carga definiciones.
  - `MissionRuntime` controla progreso, métricas y overlay.
  - `WaypointSystem` actualiza `FlightData` y renders 3D.

## 5. Directorios y Archivos Relevantes
| Ruta | Descripción |
|------|-------------|
| `src/main.cpp` | Punto de entrada, crea `core::Application`. |
| `src/core/Application.*` | Inicialización de GLFW/GLAD, recursos compartidos y bucle principal. |
| `src/states/*.cpp` | Implementaciones de Menu, Planning y Flight. |
| `src/systems/*.cpp` | Simulación física, cámara y waypoints. |
| `src/ui/*` | UI 2D: menú, planificador, overlay y HUD. |
| `assets/` | Misiones (`missions.json`), modelos, texturas y skybox. |
| `shaders/` | Shaders GLSL para HUD, modelo y terreno/skybox. |

## 6. Documentos Complementarios
- [architecture.md](architecture.md) – Profundiza en cómo se comunican los módulos (Application, estados, sistemas y render).
- [hud.md](hud.md) – Describe cada instrumento y su mapping con `FlightData`.
- [missions.md](missions.md) – Explica el flujo completo de misiones, planner, runtime y overlay.
- [shaders.md](shaders.md) – Detalle técnico de los programas GLSL utilizados.
- [assets.md](assets.md) – Estructura y gestión de recursos (modelos, texturas, fuentes).

> Esta documentación evita changelogs o auditorías antiguas; sólo cubre información vigente para desarrollar, depurar o extender el simulador.
