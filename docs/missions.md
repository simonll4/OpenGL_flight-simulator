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
El **Mission Planner** es una herramienta visual avanzada que permite editar la ruta de vuelo antes del despegue.
Utiliza fuentes TrueType (`gfx::TrueTypeFont`) para una interfaz moderna y legible.

### Paneles
1.  **Mapa (Izquierda)**: Proyección ortográfica del terreno (XZ). Permite visualizar la ruta desde arriba.
2.  **Perfil de Altitud (Inferior)**: Gráfico de Altitud vs. Distancia acumulada. Útil para detectar cambios bruscos de altura.
3.  **Storyboard (Derecha)**: Lista de tarjetas con detalles de cada waypoint (nombre, coordenadas, distancia del tramo).

### Interacción y Controles
- **Mouse**:
  - `Click Izquierdo + Arrastrar`: Mover waypoints existentes en el mapa.
  - `Click Izquierdo (en espacio vacío)`: Insertar nuevo waypoint al final de la ruta.
  - `Click Derecho`: Eliminar el waypoint bajo el cursor.
- **Teclado**:
  - `WASD`: Desplazar la vista del mapa.
  - `R`: Aumentar altitud del waypoint seleccionado.
  - `F`: Disminuir altitud del waypoint seleccionado.
  - `ENTER`: Iniciar misión (si es válida).
  - `ESC`: Regresar al menú principal.

### Generación Automática
El planificador incluye herramientas para generar patrones de vuelo predefinidos:
- **Cuadrado**: Crea 4 waypoints alrededor del centro de la vista.
- **Círculo**: Genera 6 waypoints formando un hexágono/círculo.
- **Línea**: Crea una ruta recta simple.
*(Estas funciones se invocan internamente o mediante teclas de debug si están habilitadas).*

## 3. Runtime de Misiones (`MissionRuntime`)
- **Fases**:
  - `Briefing`: Overlay visible con objetivos, física pausada.
  - `InProgress`: La simulación corre, se monitorean waypoints.
  - `Completed`: Se muestra overlay de éxito/fracaso con opciones.
  - `FreeFlight`: Permite seguir volando sin objetivos activos.
- **Métricas**:
  - Tiempo total, waypoints capturados/pendientes.
  - Velocidad media, altitud máxima.
  - Flags de estado (`menuExitRequested`, `perfectRun`, etc.).
- **Interacción con FlightState**:
  - `TAB`: Regresar al menú (marca `menuExitRequested`).
  - `M`: Saltar waypoint actual (debug/demo).
  - `R`: Reiniciar misión con los mismos parámetros.
  - `ESC`: Cerrar la aplicación.

## 4. Waypoint System (`systems/WaypointSystem`)
- Genera entidades visuales en el mundo 3D (`gfx::WaypointRenderer`).
- En cada `update`:
  - Encuentra el siguiente waypoint no capturado.
  - Calcula distancia y rumbo (bearing) y los escribe en `flight::FlightData`.
  - Verifica captura (radio configurable) y notifica a `MissionRuntime`.
- **Visualización**: Cilindros translúcidos. El waypoint activo brilla con mayor intensidad.

## 5. Overlay de Misión (`ui/overlay/MissionOverlay`)
- Se muestra en dos momentos clave:
  1.  **Briefing**: Panel central con objetivos y prompt "ENTER".
  2.  **Completion**: Resumen numérico + opciones (`TAB` menú / `SPACE` vuelo libre / `ESC` salir).
- Usa `gfx::Renderer2D` y `gfx::TextRenderer` (o TrueType si se actualiza) para mostrar información crítica sobre la vista 3D.

## 6. Controles Relacionados
| Tecla | Contexto | Acción |
|-------|----------|--------|
| `TAB` | En vuelo | Solicita regresar al menú de misiones. |
| `M` | En vuelo | Marca el waypoint actual como capturado. |
| `R` | En vuelo | Reinicia la misión actual. |
| `SPACE` | Overlay | Activa modo "Free Flight". |
| `ENTER` | Overlay | Confirma inicio o briefing. |

## 7. Extender el Sistema
- **Nuevas Misiones**: Editar `assets/missions/missions.json`.
- **Nuevos Patrones**: Extender `MissionPlanner::autoGenerateWaypoints`.
- **Validación**: Ajustar `MissionPlanner::validateMission` para reglas más estrictas (ej. altitud mínima).
