# Integración del modelo CGyAV-dlfdm en el simulador

Este documento explica:

- Qué partes del proyecto usan el modelo **CGyAV-dlfdm**
- Cómo se integra el solver `dlfdm::FDMSolver` en `flight-simulator`
- Cómo arrancar la simulación
- Cómo se generan y consumen los datos de vuelo (`FlightData`)

---

## 1. Estructura de carpetas relevante

- `CGyAV-dlfdm/`
  - Código original del modelo FDM entregado por el profesor.
  - Núcleo del modelo:
    - `include/dlfdm/defines.h` – estructuras `AircraftParameters`, `AircraftState`, `ControlInputs`
    - `include/dlfdm/fdmsolver.h` – integrador RK4 y API pública
    - `src/dlfdm/aerodynamicsmodel.cpp`
    - `src/dlfdm/aircraftdynamics.cpp`
    - `src/dlfdm/fdmsolver.cpp`
  - Ejemplos:
    - `test/main.cpp` – ejemplo de uso básico (CSV `salida.csv`)
    - `test/hud-data-example.cpp` – ejemplo de cómo derivar datos tipo HUD a partir del FDM.

- `flight-simulator/include/dlfdm/`
  - Copia del API original de `CGyAV-dlfdm`, con **solo extensiones de lectura** (getters extra).

- `flight-simulator/src/flight/dlfdm/`
  - Copia casi literal del núcleo numérico del modelo:
    - `aerodynamicsmodel.cpp`
    - `aircraftdynamics.cpp`
    - `fdmsolver.cpp`

- `flight-simulator/src/flight/`
  - `dlfdm/FdmSimulation.h/.cpp` – *wrapper de alto nivel* alrededor de `dlfdm::FDMSolver`.
  - `data/FlightData.h/.cpp` – datos de vuelo en unidades “de instrumento” para el HUD y otros sistemas.

- `flight-simulator/src/hud/core/FlightHUD.*`
  - HUD que consume `flight::FlightData` y dibuja instrumentos.

- `flight-simulator/src/main.cpp`
  - Punto de entrada, donde se conecta todo:
    - Entrada de usuario → `FdmSimulation`
    - Modelo 3D F-16
    - Terreno, skybox, HUD, waypoints, cámara.

---

## 2. ¿Sigue siendo el mismo modelo FDM?

Sí. A nivel de ecuaciones y parámetros, el modelo usado dentro de `flight-simulator` es el mismo que en `CGyAV-dlfdm`, con cambios mínimos que **no alteran la dinámica**, sólo la exposición de datos.

### 2.1 Código numérico (dinámica y aerodinámica)

Comparación entre:

- `CGyAV-dlfdm/src/dlfdm/aerodynamicsmodel.cpp`
- `flight-simulator/src/flight/dlfdm/aerodynamicsmodel.cpp`

Los archivos son **idénticos** (sin cambios).

Comparación entre:

- `CGyAV-dlfdm/src/dlfdm/aircraftdynamics.cpp`
- `flight-simulator/src/flight/dlfdm/aircraftdynamics.cpp`

La única diferencia en `flight-simulator` es la sobrecarga de `operator<<` para `glm::vec2/vec3` (para logging).  
Todas las ecuaciones:

- Transformación cuerpo ↔ NED
- Navegación (ecuaciones 2.4-5 de Stevens & Lewis)
- Dinámica traslacional y rotacional

son **idénticas** a las del modelo original.

Comparación entre:

- `CGyAV-dlfdm/src/dlfdm/fdmsolver.cpp`
- `flight-simulator/src/flight/dlfdm/fdmsolver.cpp`

Cambios en `flight-simulator`:

- Se agregan `operator<<` para `glm::vec2/vec3` (logging).
- Se añaden comentarios y formateo.
- Se añaden llamadas a:
  - `calculateAerodynamicData()`
  - `calculateGForces()`

Estas funciones calculan **solo datos derivados**:

- `aero_angles_` (ángulo de ataque y resbale, α y β)
- `dynamic_pressure_` (q = ½ ρ V²)
- `g_force_` y `g_force_components_` a partir de la aceleración.

No modifican ni el estado ni las fuerzas utilizadas en las ecuaciones de movimiento, por lo que **no cambian el modelo dinámico**, sólo exponen telemetría adicional.

La integración numérica RK4, el paso de tiempo (`1/120 s`), y las ecuaciones de actualización de estado son las mismas.

### 2.2 Parámetros del avión

En el ejemplo original (`CGyAV-dlfdm/test/main.cpp`) se usa:

- `LoadJetTrainerModel()` → llena `AircraftParameters` con:
  - `mass = 1815.0f`
  - `Ixx = 1084.6f`, `Iyy = 6507.9f`, `Izz = 7050.2f`, `Ixz = 271.16f`
  - `wingArea = 12.63f`, `wingChord = 1.64f`, `wingSpan = 8.01f`
  - `maxThrust = 11120.0f`
  - Coeficientes aerodinámicos `CL0`, `CLa`, etc.
  - Límites de superficies: mismos rangos de elevador, alerones, timón.

En el simulador (`flight-simulator/src/flight/dlfdm/FdmSimulation.cpp`), el método:

- `FdmSimulation::buildDefaultAircraft()`

reproduce **exactamente** estos mismos valores. Es decir, el avión físico utilizado es el mismo “Jet Trainer” del ejemplo del profesor.

### 2.3 Condiciones de trim e inputs

En `CGyAV-dlfdm/test/main.cpp`:

- `init_state.intertial_position = (0, 0, -5000)`   // 5000 m de altitud
- `init_state.boby_velocity = (149.998, 0, -0.36675)`  // velocidad trimada
- `controls.elevator = -0.09024`
- `controls.aileron = 0`
- `controls.rudder = 0`
- `controls.throttle = 0.3202`

En el simulador:

- `FdmSimulation::buildDefaultTrimState()` usa:
  - Misma velocidad y actitud
  - Altitud inicial diferente: `z = -1500` m (1500 m sobre el terreno)
- `FdmSimulation::buildDefaultTrimControls()` usa los **mismos** valores de trim para los controles.

La única diferencia es la **altura inicial**, que no cambia las ecuaciones del modelo (en este FDM la densidad del aire se toma constante). Se adapta para que el avión aparezca a una altitud más razonable sobre el terreno del simulador.

Conclusión: el modelo en `flight-simulator` es el mismo CGyAV-dlfdm del profesor, extendido con getters y cálculos de telemetría, pero con las mismas ecuaciones y parámetros básicos.

---

## 3. Cómo se integra el modelo en el simulador

### 3.1 Capas principales

1. **Modelo físico (CGyAV-dlfdm)**
   - Clases: `dlfdm::FDMSolver`, `dlfdm::AerodynamicsModel`, `dlfdm::AircraftDynamics`.
   - Trabajan en sistema de referencia NED:
     - Posición: `(x = north, y = east, z = down)`
     - Velocidades y ω en ejes de cuerpo.

2. **Wrapper de simulación (`flight::FdmSimulation`)**
   - Ubicación: `flight-simulator/src/flight/dlfdm/FdmSimulation.*`
   - Responsabilidades:
     - Crear y configurar `AircraftParameters`, estado de trim y controles de trim.
     - Mantener un **paso de integración fijo** (120 Hz) usando un acumulador.
     - Convertir entradas normalizadas `[-1, 1]` (joystick virtual) a deflexiones reales en radianes.
     - Convertir la salida del modelo NED → sistema de coordenadas del motor 3D.
     - Calcular `flight::FlightData` en unidades de instrumentos (ft, kt, ft/min, grados).

3. **Datos de vuelo (`flight::FlightData`)**
   - Ubicación: `flight-simulator/src/flight/FlightData.*`
   - Estructura que resume todo lo que necesita el HUD y otros sistemas:
     - Actitud: `pitch`, `roll`, `yaw`, `heading`
     - Navegación: `altitude`, `airspeed`, `verticalSpeed`
     - Posición/velocidad en mundo
     - Datos aerodinámicos: `angleOfAttack`, `sideslip`, `dynamicPressure`
     - G-forces: total, normal, lateral, `loadFactor`
     - Waypoints: `targetWaypoint`, `hasActiveWaypoint`, `waypointDistance`, `waypointBearing`

4. **HUD (`hud::FlightHUD`) y resto del simulador**
   - Consumen `FlightData` para:
     - Dibujar instrumentos de vuelo.
     - Mostrar información de navegación.
     - Ajustar cámara y otros elementos visuales.

### 3.2 Flujo de datos frame a frame

1. **Entrada de usuario** (`src/main.cpp`)
   - `processInput()` lee teclado:
     - `W/S` → pitch
     - `A/D` → roll
     - `Q/E` → yaw
     - `↑/↓` → throttle (0..1)
   - Estos valores se guardan en un **joystick virtual** (`VirtualJoystick`).

2. **Suavizado de controles**
   - `updateFlightPhysics(float dt)` aplica:
     - `smoothControlTowards()` → filtra joystick y throttle para simular actuadores con respuesta suave.
   - El resultado se pasa a:
     - `gFdmSimulation.setNormalizedInputs(elevator, aileron, rudder, throttle);`
     - Rango de entrada: `[-1, 1]` para superficies, `[0, 1]` para potencia.

3. **Integración del modelo FDM**
   - `gFdmSimulation.update(dt);` hace:
     1. Acumular `deltaTime`.
     2. Mientras `accumulator_ >= fixedTimeStep_` (1/120 s):
        - `validatePhysicalState()` – chequeos de stall, G excesivas, spin (no altera la dinámica, sólo marca flags).
        - `solver_->update(controls_);` – aquí se ejecuta el **núcleo CGyAV-dlfdm**:
          - Clamping de controles en `FDMSolver::update`.
          - Cálculo de fuerzas aerodinámicas.
          - Cálculo de derivadas de estado (`AircraftDynamics::compute_derivatives`).
          - Integración RK4.
          - Cálculo de ángulos aerodinámicos y G-forces (datos derivados).
        - Escritura opcional de logs CSV si `enableLogging()` está activo.
        - `accumulator_ -= fixedTimeStep_;`
     3. Al final del bucle fijo, llamada a `syncState()`.

4. **Sincronización con el mundo 3D y FlightData**
   - `FdmSimulation::syncState()`:
     - Lee:
       - `const AircraftState &state = solver_->getState();`
       - `const auto derivatives = solver_->get_state_dot();`
     - Construye la matriz cuerpo→NED y después NED→mundo:
       - `buildBodyToNed(state)`
       - `bodyToWorld(bodyToNed)`
     - Convierte:
       - Posición NED → posición mundo (`nedToWorld`).
       - Orientación NED → `glm::quat` de mundo.
       - True airspeed = `|boby_velocity|`.
     - Llama a `updateFlightData(...)` para rellenar `flight::FlightData`.

5. **Uso de datos en el resto del simulador**
   - En `updateFlightPhysics(dt)`:
     - `planePos = gFdmSimulation.getWorldPosition();`
     - `planeOrientation = gFdmSimulation.getWorldOrientation();`
     - `planeSpeed = gFdmSimulation.getTrueAirspeed();`
     - `flightData = gFdmSimulation.getFlightData();`
   - Estos valores se usan para:
     - Renderizar el modelo F-16 (posición/orientación).
     - Ajustar la cámara (primera/tercera persona).
     - Calcular lógica de waypoints (`updateWaypointData()`).
     - Actualizar el HUD:
       - `flightHUD.update(flightData);`
       - `flightHUD.render();`

### 3.3 Frecuencia de integración (120 Hz) vs FPS de render (~60 Hz)

- El integrador del modelo (`FDMSolver`) siempre avanza con un **paso de tiempo fijo** `fixedTimeStep_ = 1/120 s`, independiente de los FPS.
- Cada frame de render, `gFdmSimulation.update(deltaTime)`:
  - Acumula el `deltaTime` real del frame.
  - Ejecuta tantos pasos de física de 1/120 s como quepan en ese delta (a veces 1, a veces 2, etc.).
  - Al terminar, sincroniza un único estado “coherente” (`syncState()`) que se usa para dibujar el frame.
- Esto permite, por ejemplo:
  - Física a 120 Hz (estable y determinista) aunque la GPU dibuje a ~60 FPS.
  - Que variaciones leves en los FPS no cambien la solución numérica del modelo, solo la suavidad visual.
  - Que el HUD y el modelo 3D siempre vean el **último estado físico válido**, evitando “saltos” grandes aunque el frame haya tardado un poco más de lo normal.

---

## 4. Cómo se calculan los datos de FlightData

La función clave es:

- `FdmSimulation::updateFlightData(...)` en `src/flight/dlfdm/FdmSimulation.cpp`.

Principales conversiones:

- **Altitud**:
  - `altitudeMeters = -state.intertial_position.z;`
  - `altitude = altitudeMeters * 3.2808399` → pies.
  - Diferencia vs ejemplo original del profesor:
    - El ejemplo usaba un *offset* basado en la altitud de trim.
    - Aquí se usa directamente `-z` (altura sobre z=0 del mundo), más natural para el terreno del simulador.

- **Velocidad**:
  - `trueAirspeed = |boby_velocity|` en m/s.
  - `airspeed = trueAirspeed * 1.94384449` → kt.

- **Vertical Speed (variómetro)**:
  - `verticalSpeed = -derivatives.ned_position_dot.z * 196.850394` → ft/min.
  - El signo `-` hace que *subir* sea positivo (convención típica de instrumentos).
  - En el ejemplo original de HUD de CGyAV el signo era inverso; aquí se ajustó a la convención “clásica”.

- **Actitud y rumbo**:
  - Se obtienen desde el `glm::quat` de orientación en mundo:
    - `euler = glm::eulerAngles(worldOrientation_)`
    - `pitch = -deg(euler.x)`  (ajuste por convenciones de ejes)
    - `roll = deg(euler.z)`
    - `heading = norm( deg(euler.y) )` en [0, 360).
  - Esto está conceptualmente alineado con:
    - `pitch ≈ theta`, `roll ≈ phi`, `heading ≈ psi` del modelo NED.

- **Datos aerodinámicos**:
  - `angleOfAttack = deg(solver_->getAngleOfAttack());`
  - `sideslip = deg(solver_->getSideslip());`
  - `dynamicPressure = solver_->getDynamicPressure();`

- **Velocidades angulares (body)**:
  - `rollRate = deg(state.body_omega.x);`
  - `pitchRate = deg(state.body_omega.y);`
  - `yawRate = deg(state.body_omega.z);`

- **G-forces**:
  - `gForce = solver_->getGForce();`
  - `gForceNormal = gComps.z;`
  - `gForceLateral = gComps.y;`
  - `loadFactor = |gComps|;`

Estos datos están pensados para ser consumidos directamente por instrumentos y sistemas de navegación sin tener que volver a tocar el modelo numérico.

---

## 5. Cómo iniciar la simulación

### 5.1 Compilación

Desde la carpeta `flight-simulator/`:

```bash
make clean
make -j4
```

El ejecutable se genera en:

- `flight-simulator/build/FlightSim-HUD`

### 5.2 Ejecución

Desde `flight-simulator/`:

```bash
./build/FlightSim-HUD
```

Al iniciar:

- Se crea el terreno clipmap, skybox y se carga el modelo F-16.
- Se construye `gFdmSimulation` y se posiciona el avión en el estado de trim.
- Se inicializa el HUD y el sistema de waypoints.

### 5.3 Controles principales

- Vuelo:
  - `W/S` – pitch
  - `A/D` – roll
  - `Q/E` – yaw
  - `↑/↓` – throttle
- Cámara:
  - `V` – alternar vista 1ª persona / 3ª persona
  - `Z/X` – acercar/alejar cámara
  - `C` – activar/desactivar cámara suave
- Waypoints:
  - `N` – activar/desactivar sistema de waypoints
  - `M` – saltar waypoint actual
  - `R` – reiniciar misión
  - `I` – cambiar estilo visual del Waypoint Indicator

---

## 6. Logging y análisis de datos

El modelo mantiene la capacidad de loguear a CSV igual que el ejemplo original.

En `FdmSimulation`:

- `enableLogging(const std::string& filename);`
  - Abre un archivo CSV y escribe:
    - Encabezados: `solver_->log_titles(...)`
    - Luego cada paso: `solver_->log_state(...)` en `writeLogEntry()`.
- `disableLogging();`

Puedes activar el logging, por ejemplo, al iniciar el simulador (ej. en `main.cpp`):

```cpp
gFdmSimulation.initialize();
gFdmSimulation.enableLogging("fdm_output.csv");
```

Obtendrás un archivo similar al de `CGyAV-dlfdm/test`, con estados y derivadas, pero ejecutando el FDM “dentro” del simulador 3D.

---

## 7. Resumen final

- El núcleo numérico del FDM (ecuaciones y parámetros) en `flight-simulator` es **equivalente** al modelo original `CGyAV-dlfdm`.
- Las modificaciones realizadas son:
  - Extender el solver con getters y telemetría (ángulos aerodinámicos, presiones, G-forces).
  - Adaptar la altura inicial y la convención de signos de algunos datos de HUD (altitud, vertical speed) al entorno del simulador.
  - Mapear NED → sistema de coordenadas del motor 3D.
- `flight::FdmSimulation` encapsula el solver y lo conecta con:
  - Controles de usuario normalizados
  - Mundo 3D (posición/orientación del F-16)
  - Sistema de instrumentos via `flight::FlightData`.

Con esto tienes una integración completa del modelo CGyAV-dlfdm dentro del simulador OpenGL, manteniendo el modelo matemático original y exponiendo los datos de forma amigable para el HUD y la lógica de juego.
