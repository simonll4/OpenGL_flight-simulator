# Máquina de Estados de la Aplicación

La app usa una máquina de estados sencilla para orquestar flujo y UI. Cada estado implementa `states::IModeState` con tres métodos: `handleInput(AppContext&)`, `update(AppContext&)`, `render(AppContext&)`, además de `onEnter`/`onExit` para inicialización/desinicialización.

Estados disponibles (`src/states`):
- **MenuState**: Menú principal. Actualiza/renderiza `UIManager::MissionMenu`. Responde a selección de misión (Enter/Espacio) y salida (ESC). Al seleccionar, fija `context.appState = Planning` o `Running` según flujo.
- **PlanningState**: Planificador de ruta. Usa `UIManager::MissionPlanner` y `MissionController` para editar/cargar la misión seleccionada. Al aceptar o cancelar, fija el siguiente estado (`Running` o `Menu`).
- **FlightState**: Simulación en vuelo. Orquesta física (`FlightSimulationController`), cámara (`CameraRig`), waypoints, HUD y terreno/skybox. Al pedir salir o al mostrar overlay, puede cambiar a `Menu` o finalizar la app.

Transiciones:
- `Application::mainLoop` llama `transitionIfNeeded()`, que compara `context.appState` con el estado activo y realiza `onExit` del actual y `onEnter` del nuevo.
- Estados actualizan `context.appState` cuando el usuario actúa (ej. MenuState al seleccionar misión, PlanningState al aceptar/cancelar, FlightState al salir).
- `AppState::Exiting` provoca cierre de ventana desde `Application`.

Contexto compartido:
- `AppContext` porta punteros a controladores (misiones, UI, física, cámara, waypoints) y recursos gráficos (terreno, skybox, shaders, modelo), además de timing y tamaño de pantalla. Se pasa a cada estado en `handleInput/update/render`.

Entrada y rendering:
- Cada iteración del loop principal delega a `activeState_` las tres fases (input, update, render) usando el mismo `AppContext`, lo que garantiza coherencia y evita singletons.

Archivos clave:
- `src/core/Application.cpp` — creación de estados, bucle principal y transición.
- `src/states/MenuState.*`, `PlanningState.*`, `FlightState.*` — lógica específica de cada modo.
