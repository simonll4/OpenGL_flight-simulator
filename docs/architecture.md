# Arquitectura del Simulador

Este documento resume cómo se organiza el código y qué responsabilidades tiene cada capa dentro del Flight Simulator HUD.

## 1. Núcleo (`src/core`)
- **`core::Application`**
  - Inicializa GLFW/GLAD, ventana y recursos OpenGL (terreno plano, skybox, shaders, modelo).
  - Mantiene un `core::AppContext` con punteros a todos los subsistemas.
  - Gestiona un mapa de estados (`states::IModeState`) y delega `handleInput/update/render`.
  - Controla el ciclo principal (timing, resize, swap buffers, transición de estados y shutdown).

## 2. Estados (`src/states`)
| Estado | Responsable | Notas |
|--------|-------------|-------|
| `MenuState` | Actualiza `UIManager::MissionMenu`, selecciona misión y guarda `MenuStatePersistence`. | Preserva última selección al relanzar. |
| `PlanningState` | Coordina el planificador (`ui::MissionPlanner`) y lanza la simulación. | Carga la misión en `WaypointSystem`, resetea cámara y controlador. |
| `FlightState` | Ejecuta simulación, actualiza cámara/HUD/waypoints y overlay. | Gestiona entrada (ESC, TAB, M, R), solicita regreso al menú y dibuja escena 3D. |

## 3. Sistemas (`src/systems`)
- **FlightSimulationController**
  - Envoltorio del solver DLFDM (`flight/dlfdm`).
  - Lee teclado (WASD/QE + flechas) y ajusta `ControlInputs`.
  - Después de `step` expone posición, orientación y `flight::FlightData`.
- **CameraRig**
  - Primera/tercera persona, zoom (`Z/X`) y smoothing (`C`).
  - Ajusta `view` y `projection` dependiendo de altura para fijar el far plane.
- **WaypointSystem**
  - Carga waypoints desde `mission::MissionDefinition`.
  - Actualiza `FlightData` (bearing, distancia, `hasActiveWaypoint`).
  - Renderiza cilindros 3D con `gfx::WaypointRenderer`.

## 4. Renderizado (`src/gfx`)
- **Terreno**: `TerrainPlane` dibuja un plano texturizado (grid 3x3 alrededor de la cámara) repetido con `GL_REPEAT` para no dejar bordes visibles, sin heightmap/LOD.
- **Skybox**: `SkyboxRenderer + TextureCube`.
- **Modelo**: `gfx::Shader + gfx::Model` para el F-16 (Assimp + PBR básico).
- **Texto**:
  - **`gfx::TextRenderer`**: Renderizado estilo display de 7 segmentos (usado en HUD).
  - **`gfx::TrueTypeFont`**: Renderizado de fuentes vectoriales de alta calidad usando `stb_truetype` (usado en Mission Planner y UI moderna).
- **Shaders**: Gestión de programas GLSL (ver `docs/shaders.md`).

## 5. Interfaz de Usuario (`src/ui`)
- **UIManager**
  - Crea y actualiza `MissionMenu`, `MissionPlanner`, `MissionOverlay` y `hud::FlightHUD`.
  - Expone helpers `updateMenu`, `renderPlanner`, `updateHUD`, `renderOverlay`, etc.
- **MissionMenu**
  - Lista misiones con `Renderer2D`.
  - Devuelve `MenuResult` con `missionSelected` / `exitRequested`.
- **MissionPlanner**
  - Editor visual completo (mapa XZ + storyboard + perfil de altitud).
  - Utiliza `TrueTypeFont` para textos legibles.
  - Permite mover waypoints, generar patrones automáticos y visualizar métricas de misión.
- **MissionOverlay**
  - Panel modal durante briefing/completion (ver `docs/missions.md`).

## 6. HUD (`src/hud`)
- `hud::FlightHUD` instancia instrumentos y comparte el mismo `Renderer2D`.
- Cada instrumento hereda de `hud::Instrument` y consume `flight::FlightData`.
- Instrumentos actuales: SpeedIndicator, Altimeter, VerticalSpeedIndicator, WaypointIndicator, BankAngleIndicator y PitchLadder. Detalle en `docs/hud.md`.

## 7. Datos y Misiones (`src/mission`)
- **MissionRegistry**: parser JSON ligero para `assets/missions/missions.json`.
- **MissionController**: expone `registry`, `runtime` y misión activa.
- **MissionRuntime**:
  - Controla fases (Briefing, InProgress, Completed, FreeFlight).
  - Lleva métricas (tiempo, waypoints capturados, velocidad media, altitud).
  - Recibe datos de `FlightSimulationController` para detectar capturas (`WaypointSystem`) y disparar overlay.

## 8. Flujo de Datos Principal
```
DLFDM Solver → FlightSimulationController
         ↓
   flight::FlightData
         ↓
 HUD / MissionOverlay / WaypointSystem / MissionRuntime
```
- `FlightData` es la única fuente de verdad para instrumentos y navegación.
- `MissionRuntime` consulta `FlightData` en cada frame para actualizar progreso.
- `UIManager` vuelve a enviar `FlightData` al HUD y al overlay (resumen de misión).

## 9. Consideraciones de Implementación
- Los recursos GL (shaders, buffers, texturas) se inicializan una vez en `Application::initResources`.
- Todas las clases con recursos GL proporcionan `init()` y `cleanup()` para permitir reinicializaciones controladas.
- El HUD sólo se dibuja cuando la cámara está en primera persona (`CameraRig::isFirstPerson()`).
- El overlay se dibuja al final del `FlightState::render` con profundidad deshabilitada.
