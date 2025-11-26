# Documentación de Shaders

Este documento describe los shaders GLSL utilizados en el proyecto, ubicados en el directorio `shaders/`.

## 1. Terreno (`clipmap.vert`, `clipmap.frag`)
Renderiza el terreno utilizando la técnica de **Geometry Clipmaps** para un nivel de detalle (LOD) dinámico y eficiente.

### Vertex Shader (`clipmap.vert`)
- **Entradas**: Posición del vértice (`a_Pos`).
- **Uniforms**: Matrices de transformación, texturas de heightmap/normalmap, parámetros de clipmap (escala, nivel, offset de altura).
- **Proceso**:
  - Calcula la posición mundial basada en el modelo.
  - Mapea la posición mundial a coordenadas UV para leer el heightmap.
  - (Nota: En la implementación actual, la altura se fija en 0.0 y la normal en (0,1,0) para un terreno plano, aunque la lógica para leer el heightmap está presente pero no se aplica al `gl_Position` final en este shader específico, posiblemente para simplificación o debug).
  - Calcula el nivel de LOD para visualización de depuración.

### Fragment Shader (`clipmap.frag`)
- **Uniforms**: Texturas (heightmap, normalmap, textura de terreno), color de fondo (niebla), posición de cámara.
- **Proceso**:
  - Aplica una textura de terreno con tiling.
  - Calcula iluminación direccional simple (Lambert).
  - Aplica niebla lineal basada en la distancia a la cámara.
  - Permite visualizar niveles de LOD mediante colores (verde a rojo) si `u_ShowLODColors` está activo.

## 2. Terreno Detallado (`terrain.vert`, `terrain.frag`)
Una alternativa para renderizado de terreno con mapeo triplanar y texturas de detalle.

### Vertex Shader (`terrain.vert`)
- **Entradas**: Posición y normal.
- **Uniforms**: Matriz ViewProjection, offset de grid (para snapping).
- **Proceso**: Desplaza los vértices según el offset del grid para acompañar a la cámara.

### Fragment Shader (`terrain.frag`)
- **Uniforms**: Texturas (albedo, normal, roughness, detalle), tintes, configuración de tiling.
- **Proceso**:
  - **Triplanar Mapping**: Proyecta texturas en los ejes X, Y, Z y las mezcla según la normal del terreno para evitar distorsión en pendientes.
  - **Detalle**: Mezcla una textura de detalle sobre la textura base.
  - **Iluminación**: Lambert + Ambient + Roughness.
  - **Niebla Exponencial**: Para suavizar el horizonte.

## 3. Modelo (`model.vert`, `model.frag`)
Shader estándar para renderizar objetos 3D (como el avión F-16).

### Vertex Shader (`model.vert`)
- **Entradas**: Posición, normal, coordenadas de textura, tangente, bitangente.
- **Proceso**: Transforma vértices y normales al espacio mundial.

### Fragment Shader (`model.frag`)
- **Uniforms**: Textura difusa, propiedades de luz (posición, color), posición de vista.
- **Proceso**:
  - Implementa el modelo de iluminación **Blinn-Phong** (Ambiente + Difuso + Especular).
  - Aplica corrección Gamma.
  - Soporta texturas difusas o un color gris por defecto.

## 4. Skybox (`skybox.vert`, `skybox.frag`)
Renderiza el fondo del entorno utilizando un Cube Map.

### Vertex Shader (`skybox.vert`)
- **Proceso**: Escribe la posición en `gl_Position` con `z = w` para asegurar que el skybox se renderice siempre en la profundidad máxima (detrás de todo).

### Fragment Shader (`skybox.frag`)
- **Proceso**: Muestrea un `samplerCube` para obtener el color del cielo.

## 5. HUD (`hud.vert`, `hud.frag`)
Renderizado 2D para la interfaz de usuario y el Head-Up Display.

### Vertex Shader (`hud.vert`)
- **Entradas**: Posición 2D, color, coordenadas de textura.
- **Proceso**: Proyección ortográfica simple.

### Fragment Shader (`hud.frag`)
- **Uniforms**: Textura opcional (`uUseTexture`).
- **Proceso**:
  - Si se usa textura (ej. para texto), utiliza el canal rojo como alpha/cobertura.
  - Aplica `smoothstep` para suavizar bordes de texto (anti-aliasing simple).
  - Multiplica el color del vértice por la textura.

## 6. Waypoints (`waypoint.vert`, `waypoint.frag`)
Renderiza los marcadores visuales de los waypoints en el mundo 3D.

### Vertex Shader (`waypoint.vert`)
- **Proceso**: Transformación estándar MVP. Calcula normales para iluminación.

### Fragment Shader (`waypoint.frag`)
- **Uniforms**: Color, alpha, estado activo (`isActive`).
- **Proceso**:
  - Iluminación difusa simple.
  - Añade un efecto de brillo (`glow`) si el waypoint es el objetivo activo.
  - Renderiza con transparencia.
