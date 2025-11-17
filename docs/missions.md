# Sistema de Misiones y Navegación

Este documento detalla cómo se cargan las misiones, cómo se planifica una ruta y qué componentes intervienen durante la simulación.

## 1. Definiciones de Misión (`mission/*.h`)
- **`MissionDefinition`** describe:
  - `id`, `name`, `description`, `briefing`.
  - `startPosition`, `startOrientation`.
  - `std::vector<WaypointDef>` con `position` y `name`.
  - `EnvironmentSettings` (hora del día, clima, viento) y metadata (`category`, `difficulty`).
- **`MissionRegistry`**:
  - Carga `assets/missions/missions.json` con un parser sencillo.
  - Expone `getMissionByIndex` para el menú.
- **`MissionController`**:
  - Mantiene `registry`, `runtime` y la misión activa.
  - Persiste la última selección (`MenuStatePersistence` → `menu_state.json`).

## 2. Planificador (`ui/menu/MissionPlanner.*`)
- Renderiza 3 paneles:
  - **Mapa**: proyección ortográfica de `X/Z`, admite arrastrar waypoints.
  - **Perfil**: altura vs distancia acumulada.
  - **Storyboard**: tarjetas resumidas de cada waypoint.
- Acciones soportadas:
  - `click + drag`: mover waypoints.
  - `R/F`: ajustar altitud del waypoint seleccionado.
  - Botón “START MISSION”: finaliza el planner y cambia a `FlightState`.

## 3. Runtime de Misiones (`MissionRuntime`)
- **Fases**:
  - `Briefing`: overlay visible, física pausada.
  - `InProgress`: la simulación corre, se monitorean waypoints.
  - `Completed`: se muestra overlay para elegir (menú ↔ vuelo libre).
  - `FreeFlight`: permite seguir volando sin waypoints activos.
- **Métricas**:
  - Tiempo total, waypoints capturados/pendientes.
  - Velocidad media, altitud máxima.
  - Flags de estado (`menuExitRequested`, `perfectRun`, etc.).
- **Interacción con FlightState**:
  - `TAB`: regresar al menú (marca `menuExitRequested`).
  - `M`: saltar waypoint actual (para depuración o demos).
  - `R`: reiniciar misión con los mismos parámetros.
  - `ESC`: cerrar la aplicación.

## 4. Waypoint System (`systems/WaypointSystem`)
- Genera entradas internas (`position`, `name`, `captured`).
- En cada `update`:
  - Encuentra el siguiente waypoint no capturado.
  - Calcula distancia/bearing y los escribe en `flight::FlightData`.
  - Verifica captura (radio configurable) y notifica a `MissionRuntime`.
- `render` dibuja cilindros 3D; el waypoint activo se resalta en verde brillante.

## 5. Overlay de Misión (`ui/overlay/MissionOverlay`)
- Se muestra en dos momentos:
  1. **Briefing**: panel central con objetivos y un prompt “ENTER”.
  2. **Completion**: resumen numérico + opciones (`TAB` volver al menú / `SPACE` continuar en vuelo libre / `ESC` salir).
- Usa `gfx::Renderer2D` + `TextRenderer`.
- `FlightState::render` deshabilita depth antes de llamar a `renderOverlay`.

## 6. Controles Relacionados
| Tecla | Contexto | Acción |
|-------|----------|--------|
| `TAB` | En vuelo | Solicita regresar al menú de misiones. |
| `M` | En vuelo | Marca el waypoint actual como capturado. |
| `R` | En vuelo | Reinicia la misión actual. |
| `SPACE` | Overlay de completion | Activa modo “Free Flight”. |
| `ENTER` | Overlay de briefing | Confirma que el piloto está listo para despegar. |

## 7. Extender el Sistema
- Para nuevas misiones:
  1. Añadir entradas en `assets/missions/missions.json`.
  2. Ajustar `MissionPlanner` si se necesitan campos adicionales.
  3. Documentar los parámetros relevantes en `MissionDefinition`.
- Para nuevas reglas de progreso:
  - Extender `MissionRuntime::updateProgress` (ej. radio dinámico, objetivos por altitud).
  - Propagar los nuevos campos a `MissionOverlay::formatMetrics`.
- Para overlays personalizados:
  - Reutilizar `MissionOverlay::drawBox`/`drawBackground`.
  - Añadir estados adicionales (por ejemplo, pausa) siguiendo la misma interfaz.
