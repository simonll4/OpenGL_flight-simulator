# Documentación de Shaders

Este documento describe los shaders GLSL utilizados en el proyecto, ubicados en el directorio `shaders/`.

## 1. Terreno Plano (`terrain_plane.vert`, `terrain_plane.frag`)
Renderiza un plano texturizado que se desplaza con la cámara. No usa heightmap ni niveles de detalle.

### Vertex Shader (`terrain_plane.vert`)
- **Entradas**: Posición del vértice (`a_Pos`).
- **Uniforms**: Matrices de transformación, tamaño del tile (`u_TileSize`) y factor de tiling (`u_TextureTiling`).
- **Proceso**:
  - Calcula posición en mundo con el modelo (Y=0 siempre).
  - Usa una normal fija `(0,1,0)`.
  - Genera coordenadas UV a partir de la posición mundial y el factor de tiling.

### Fragment Shader (`terrain_plane.frag`)
- **Uniforms**: Textura de color, color de fondo (para niebla), posición de cámara.
- **Proceso**:
  - Samplea la textura con `GL_REPEAT`.
  - Calcula iluminación direccional simple (Lambert).
  - Aplica niebla lineal basada en distancia.

## 2. Modelo (`model.vert`, `model.frag`)
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

## 3. Skybox (`skybox.vert`, `skybox.frag`)
Renderiza el fondo del entorno utilizando un Cube Map.

### Vertex Shader (`skybox.vert`)
- **Proceso**: Escribe la posición en `gl_Position` con `z = w` para asegurar que el skybox se renderice siempre en la profundidad máxima (detrás de todo).

### Fragment Shader (`skybox.frag`)
- **Proceso**: Muestrea un `samplerCube` para obtener el color del cielo.

## 4. HUD (`hud.vert`, `hud.frag`)
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

## 5. Waypoints (`waypoint.vert`, `waypoint.frag`)
Renderiza los marcadores visuales de los waypoints en el mundo 3D.

### Vertex Shader (`waypoint.vert`)
- **Proceso**: Transformación estándar MVP. Calcula normales para iluminación.

### Fragment Shader (`waypoint.frag`)
- **Uniforms**: Color, alpha, estado activo (`isActive`).
- **Proceso**:
  - Iluminación difusa simple.
  - Añade un efecto de brillo (`glow`) si el waypoint es el objetivo activo.
  - Renderiza con transparencia.
