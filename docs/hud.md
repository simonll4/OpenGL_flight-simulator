# HUD e Instrumentación

El HUD se implementa en `src/hud` y utiliza `gfx::Renderer2D` para dibujar un overlay monocromático inspirado en el F‑16. Todos los instrumentos consumen `flight::FlightData`, lo que facilita pruebas y mocking.

## 1. Flujo General
1. `UIManager` inicializa `hud::FlightHUD` y le pasa el ancho/alto de pantalla.
2. `FlightHUD::update` recibe un `flight::FlightData` por frame y lo guarda.
3. `FlightHUD::render`:
   - Activa blending y deshabilita depth.
   - Llama a `Instrument::render` para cada elemento habilitado.
   - Restaura el estado GL.

## 2. Instrumentos Disponibles

| Instrumento | Archivo | Datos usados | Descripción |
|-------------|---------|--------------|-------------|
| **SpeedIndicator** | `hud/instruments/flight/SpeedIndicator.*` | `airspeed` | Tape vertical izquierdo a pasos de 10 kt con display digital y chevron. |
| **Altimeter** | `hud/instruments/flight/Altimeter.*` | `altitude` | Tape vertical derecho cada 100 ft con lectura central y marca lateral. |
| **VerticalSpeedIndicator** | `hud/instruments/flight/VerticalSpeedIndicator.*` | `verticalSpeed` | Escala fija ±6000 ft/min, indicador triangular y display compactado (/100). |
| **WaypointIndicator (HSI)** | `hud/instruments/navigation/WaypointIndicator.*` | `heading`, `waypointBearing`, `targetWaypoint`, `position` | Rosa de 360° con flecha magenta y medidor vertical de diferencia de altitud respecto al waypoint activo. |
| **BankAngleIndicator** | `hud/instruments/attitude/BankAngleIndicator.*` | `roll` | Línea inclinada con 5 marcas móviles y aguja fija. Diseño refinado con pendiente y grosor ajustados. |
| **PitchLadder** | `hud/instruments/attitude/PitchLadder.*` | `pitch`, `roll` | Escalera central con líneas cada 10°, mira fija y marcadores verticales. Rota con el ángulo de alabeo. |

## 3. Convenciones de Diseño
- **Color**: `glm::vec4(0.0f, 1.0f, 0.4f, 0.95f)` para mantener el aspecto HUD clásico.
- **Unidades**:
  - `airspeed`: nudos.
  - `altitude`: pies con offset de nivel de ojos (FlightData lo normaliza).
  - `verticalSpeed`: ft/min, instrumentado como ft/min/100 para display.
  - `heading`, `pitch`, `roll`: grados.
  - `waypointDistance`: metros, convertidos a etiquetas dentro de cada instrumento.
- **Renderizado**: cada instrumento obtiene `position` y `size` en píxeles desde `FlightHUD::setupInstrumentLayout`.

## 4. Extensión del HUD
1. Crear una clase derivada de `hud::Instrument`.
2. Implementar `render(Renderer2D&, const FlightData&)`.
3. Agregar la instancia en `FlightHUD` (constructor y `setupInstrumentLayout`).
4. Registrar color/posición/tamaño en `setupInstrumentLayout`.
5. (Opcional) Añadir configuración en `FlightHUD::setLayout`.

## 5. Datos Relevantes de `flight::FlightData`
- `pitch`, `roll`, `heading`, `yaw`.
- `airspeed`, `altitude`, `verticalSpeed`.
- `position` y `velocity` en metros / m/s.
- `targetWaypoint`, `waypointDistance`, `waypointBearing`, `hasActiveWaypoint`.
- Base ortonormal de la cámara (`cameraFront`, `cameraUp`, `cameraRight`).

## 6. Consejos para Nuevos Instrumentos
- Usa `renderer.drawLine/drawRect/drawCircle/drawTriangle` para componer elementos básicos.
- Para texto tabular, usa `gfx::TextRenderer::drawString` y piensa en términos de posición central en vez de esquina superior izquierda.
- Al manipular ángulos, normaliza a rangos estándar para evitar saltos (ver `FlightData::updateFromCamera` para ejemplos).
- Mantén los cálculos dependientes de datos en la sección `update`, y deja `render` exclusivamente para dibujar lo ya calculado.
