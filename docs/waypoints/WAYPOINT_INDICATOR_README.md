# Waypoint Indicator - Manual de Usuario

## Descripcion General

El **Waypoint Indicator** es un instrumento de navegacion profesional estilo HUD militar que proporciona ayuda visual para seguir rutas predefinidas mediante waypoints. Inspirado en los sistemas HSI (Horizontal Situation Indicator) de aviacion real, muestra toda la informacion necesaria para mantener la aeronave en ruta sin consultar mapas.

---

## Diseno del Instrumento

El indicador esta dividido en **dos secciones principales**:

- **Panel Izquierdo**: Brujula circular con flecha direccional
- **Panel Derecho**: Informacion digital numerica

### Especificaciones Visuales
- **Tamano**: 280x140 pixeles
- **Posicion**: Centro superior de la pantalla
- **Estilo**: HUD militar verde
- **Fondo**: Semi-transparente negro (60% opacidad)
- **Visible**: Solo en vista POV (primera persona)

---

## Panel Izquierdo - Brujula Circular

### Elementos de la Brujula

#### 1. Circulos Concentricos
- **Circulo exterior**: Limite de la brujula (radio 45px)
- **Circulo interior**: Referencia adicional (radio 34px)
- Ambos en color verde HUD

#### 2. Marcas Cardinales
- **N** en la parte superior: Indica el Norte
- **4 marcas** en los puntos cardinales (N, S, E, W)
- Lineas cortas de 5px desde el borde

#### 3. Flecha Direccional - ELEMENTO PRINCIPAL
- **Color**: Verde brillante
- **Grosor**: 4 pixeles (muy visible)
- **Longitud**: 35 pixeles desde el centro
- **Punta**: Triangulo solido de 14px
- **Funcion**: Apunta SIEMPRE hacia el waypoint objetivo

#### 4. Indicador de Rumbo Actual
- **Color**: Amarillo
- **Posicion**: Tick vertical fijo en la parte superior
- **Grosor**: 3 pixeles
- **Funcion**: Muestra hacia donde apunta tu avion

### Como Interpretar la Brujula

| Posicion de la Flecha | Significado |
|----------------------|-------------|
| Arriba | Waypoint directamente adelante - NO GIRAR |
| Derecha | Girar a la DERECHA |
| Izquierda | Girar a la IZQUIERDA |
| Abajo | Waypoint detras - Giro de 180 grados necesario |

**Consejo**: Intenta alinear la flecha con el tick amarillo superior para volar directo al waypoint.

---

## Panel Derecho - Informacion Digital

### 1. DIST - Distancia al Waypoint

Muestra que tan lejos esta el waypoint objetivo.

**Formato**:
- Mayor o igual a 1000m: En kilometros con 1 decimal (ej: "1.5 KM")
- Menor a 1000m: En metros enteros (ej: "850 M")

**Caracteristicas**:
- Numeros GRANDES (11x16px) - Facil lectura
- Actualizacion en tiempo real
- Precision: +/- 1 metro

**Ejemplo visual**:
```
DIST
1.5 KM
```

---

### 2. TURN - Angulo de Giro Requerido

Indica cuantos grados debes girar para apuntar al waypoint.

**Formato**:
- Con signo: "+" (derecha) o "-" (izquierda)
- Ejemplo: "+35 grados" = Girar 35 grados a la derecha
- Ejemplo: "-12 grados" = Girar 12 grados a la izquierda

**Codificacion por Color**:
- **Verde**: Desviacion menor a 10 grados (estas casi alineado)
- **Amarillo**: Desviacion mayor o igual a 10 grados (necesitas corregir)

**Logica**:
```
Angulo = Bearing al Waypoint - Tu Rumbo Actual
Normalizado al rango [-180, +180]
```

**Ejemplo visual**:
```
TURN
+35°    <- Amarillo (necesita correccion)
```

```
TURN
-5°     <- Verde (casi alineado)
```

---

### 3. BRG - Bearing (Rumbo al Waypoint)

Muestra el rumbo absoluto desde el Norte geografico hacia el waypoint.

**Formato**:
- 3 digitos con ceros a la izquierda
- Rango: 000° - 360°
- Ejemplo: "045°" (Noreste), "270°" (Oeste)

**Sistema de Coordenadas**:
- **000°**: Norte (-Z en el mundo 3D)
- **090°**: Este (+X en el mundo 3D)
- **180°**: Sur (+Z en el mundo 3D)
- **270°**: Oeste (-X en el mundo 3D)

**Uso**:
- Util para navegacion precisa
- Permite reportar posiciones ("Waypoint al bearing 045")
- Compatible con brujulas magneticas

**Ejemplo visual**:
```
BRG
045°
```

---

### 4. Barra de Proximidad

Indicador visual del progreso hacia el waypoint.

**Caracteristicas**:
- **Ancho**: 100 pixeles
- **Alto**: 6 pixeles
- **Color fondo**: Verde tenue (20% opacidad)
- **Color relleno**: Verde brillante (100% opacidad)

**Calculo del Relleno**:
```
Porcentaje = 1 - (Distancia / 2000m)
Limitado entre 0% y 100%
```

**Interpretacion**:
- **Vacia**: Mas de 2000m de distancia
- **25% llena**: ~1500m de distancia
- **50% llena**: ~1000m de distancia
- **75% llena**: ~500m de distancia
- **100% llena**: Menos de 100m (muy cerca!)

**Ejemplo visual**:
```
[████████░░]  <- 80% lleno = ~400m de distancia
```

---

## Estados del Indicador

### Estado ACTIVO - Waypoint Disponible

Cuando hay un waypoint activo, el indicador muestra:
- Brujula con flecha apuntando al objetivo
- Todas las metricas actualizadas en tiempo real
- Barra de proximidad activa

### Estado INACTIVO - Sin Waypoint

Cuando no hay waypoint activo (mision completada o desactivada):

**Display**:
```
┌──────────────────────────┐
│                          │
│      NO WAYPOINT         │
│          ○               │
│                          │
└──────────────────────────┘
```

- Circulo tenue en el centro
- Mensaje "NO WAYPOINT"
- Opacidad reducida (60%)

---

## Ejemplos de Uso Practico

### Escenario 1: Vuelo Directo

**Situacion**: Waypoint a 1200m directamente adelante

```
Indicador muestra:
- Flecha: Apuntando ARRIBA
- DIST: 1.2 KM
- TURN: +2° (verde)
- BRG: 045°
- Barra: [██████████] 40% llena

Accion: Mantener rumbo, no girar
```

---

### Escenario 2: Correccion a la Derecha

**Situacion**: Waypoint a 850m, necesitas girar 35° a la derecha

```
Indicador muestra:
- Flecha: Apuntando DERECHA (4 o'clock)
- DIST: 850 M
- TURN: +35° (amarillo)
- BRG: 120°
- Barra: [████████░░] 57% llena

Accion: Girar suavemente a la derecha hasta que TURN este cerca de 0°
```

---

### Escenario 3: Waypoint Muy Cercano

**Situacion**: Waypoint a 95m casi adelante

```
Indicador muestra:
- Flecha: Apuntando ARRIBA
- DIST: 95 M
- TURN: -3° (verde)
- BRG: 180°
- Barra: [██████████] 95% llena

Accion: Mantener rumbo, prepararse para captura automatica (radio 80m)
```

---

### Escenario 4: Waypoint Atras

**Situacion**: Waypoint a 1500m detras de ti

```
Indicador muestra:
- Flecha: Apuntando ABAJO
- DIST: 1.5 KM
- TURN: +180° (amarillo) o -180° (amarillo)
- BRG: 225°
- Barra: [██░░░░░░░░] 25% llena

Accion: Hacer giro de 180° en cualquier direccion
```

---

## Integracion con el Sistema de Waypoints

### Calculo de Metricas

El sistema calcula automaticamente:

1. **Distancia**:
```cpp
glm::vec3 toWaypoint = waypoint.position - planePos;
float distance = glm::length(toWaypoint);
```

2. **Bearing**:
```cpp
glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);
float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180/PI);
// Normalizado a [0, 360]
```

3. **Angulo Relativo**:
```cpp
float relativeAngle = bearing - heading;
// Normalizado a [-180, +180]
```

### Captura de Waypoints

- **Radio de captura**: 80 metros
- **Deteccion**: Automatica cada frame
- **Efecto**: El waypoint desaparece y avanza al siguiente

---

## Tecnicas de Navegacion

### Vuelo Directo (Straight Flight)

**Objetivo**: Mantener la flecha apuntando arriba

**Tecnica**:
1. Observa el valor TURN
2. Si es positivo, gira ligeramente a la derecha
3. Si es negativo, gira ligeramente a la izquierda
4. Busca mantener TURN entre -5° y +5° (verde)

---

### Interceptacion Angular (Angled Intercept)

**Objetivo**: Corregir rumbo con desviacion grande

**Tecnica**:
1. Si TURN muestra +45°, NO gires 45° instantaneamente
2. Aplica giros de ~15-20° y observa como cambia TURN
3. Ajusta gradualmente hasta que TURN este cerca de 0°
4. Evita sobrecorregir (oscilaciones)

---

### Aproximacion Final (Final Approach)

**Cuando DIST < 200m**:

1. **Reduce velocidad** si es posible
2. Mantener TURN muy pequeno (verde)
3. Observar barra de proximidad (debe estar >90%)
4. Prepararse para siguiente waypoint

---

## Configuracion Tecnica

### Archivos del Instrumento

**Header**:
```
src/hud/WaypointIndicator.h
```

**Implementacion**:
```
src/hud/WaypointIndicator.cpp
```

**Integracion**:
```
src/hud/FlightHUD.cpp  (layout y posicionamiento)
src/main.cpp           (logica de waypoints)
```

### Parametros Configurables

En `WaypointIndicator.cpp`:

```cpp
static const float PANEL_WIDTH = 280.0f;      // Ancho del panel
static const float PANEL_HEIGHT = 140.0f;     // Alto del panel
static const float COMPASS_RADIUS = 45.0f;    // Radio brujula
static const float ARROW_LENGTH = 35.0f;      // Longitud flecha
```

En `main.cpp`:

```cpp
const float WAYPOINT_CAPTURE_RADIUS = 80.0f;  // Radio de captura
```

---

## Preguntas Frecuentes (FAQ)

### ¿Por que la flecha no apunta al Norte?

La flecha NO apunta al Norte geografico. Apunta al waypoint objetivo en relacion a tu rumbo actual. Es un indicador RELATIVO, no absoluto.

### ¿Como se que estoy volando correctamente?

Cuando:
- La flecha apunta ARRIBA
- TURN muestra un valor verde (< 10°)
- La barra de proximidad aumenta constantemente

### ¿Que significa cuando TURN es amarillo?

Significa que tu desviacion es mayor a 10°. Necesitas hacer una correccion mas significativa de rumbo.

### ¿Por que desaparecen los waypoints?

Los waypoints desaparecen cuando los capturas (pasas a menos de 80m). Esto indica progreso en la mision.

### ¿Como reinicio la mision si fallo?

Presiona la tecla **R** para reiniciar todos los waypoints.

### ¿El indicador funciona en vista de tercera persona?

No, el HUD (incluyendo el waypoint indicator) solo es visible en vista POV (primera persona). Presiona **V** para cambiar de vista.

---

## Glosario de Terminos

- **HSI**: Horizontal Situation Indicator - Instrumento de navegacion en aviones reales
- **Bearing**: Rumbo absoluto desde el Norte hacia un objetivo
- **Heading**: Rumbo actual del avion
- **Turn**: Angulo de giro necesario para alinearse con el objetivo
- **POV**: Point of View - Vista en primera persona
- **HUD**: Heads-Up Display - Pantalla de visualizacion frontal
- **Waypoint**: Punto de navegacion en el espacio 3D
- **Capture**: Momento en que el avion llega al radio del waypoint

---

## Referencias y Recursos

### Sistemas Reales de Aviacion

Este instrumento esta inspirado en:
- **HSI (Horizontal Situation Indicator)**: Usado en aviacion comercial y militar
- **CDI (Course Deviation Indicator)**: Parte de sistemas de navegacion VOR/ILS
- **GPS Moving Map**: Sistemas modernos de navegacion por satelite

### Diferencias con Sistemas Reales

**Simplificaciones**:
- No hay correccion de viento
- No hay variacion magnetica
- La captura es instantanea (en aviones reales hay secuencias)
- No hay procedimientos de holding

**Ventajas del Simulador**:
- Feedback visual inmediato
- Sin retrasos de actualizacion
- Precision perfecta del GPS

---

## Creditos

**Diseno**: Basado en HSI de aviacion militar
**Implementacion**: Sistema modular con Renderer2D
**Estilo Visual**: HUD verde militar clasico

---

## Version

**Version**: 2.0 - Profesional  
**Fecha**: 2025  
**Ultima actualizacion**: Rediseno completo con panel dual

---

## Soporte

Para reportar problemas o sugerencias:
- Revisa el codigo en `src/hud/WaypointIndicator.cpp`
- Consulta `WAYPOINT_NAVIGATION.md` para el sistema completo
- Verifica configuracion en `FlightHUD.cpp`
