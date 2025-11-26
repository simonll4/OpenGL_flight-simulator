# Análisis de Shaders (GLSL)

## 1. Gestión de Shaders (`src/gfx/core/Shader`)

La clase `Shader` abstrae la complejidad de OpenGL:
1.  Lee el código fuente de archivos `.vert` y `.frag`.
2.  Compila cada uno (`glCompileShader`).
3.  Linkea el programa (`glLinkProgram`).
4.  Provee métodos para enviar **Uniforms** (`setMat4`, `setVec3`, `setFloat`).

## 2. Shader de Terreno Plano (`terrain_plane.vert` / `terrain_plane.frag`)

Plano texturizado que sigue a la cámara (sin heightmap ni LOD).

### Vertex Shader (`terrain_plane.vert`)
*   **Normal fija**: `(0,1,0)` para un suelo plano.
*   **UVs**: Se derivan de la posición mundial y un factor de tiling (`u_TextureTiling / u_TileSize`).
*   **MVP**: Transforma la posición del grid al clip space.

### Fragment Shader (`terrain_plane.frag`)
*   **Texturizado**: Usa `u_Texture` con `GL_REPEAT` para repetir el patrón.
*   **Iluminación**: Lambert simple con dirección fija del “sol”.
*   **Fog**: Factor lineal entre `u_FogMinDist` y `u_FogMaxDist`, mezclando con `u_Background`.

## 3. Shader de Skybox (`skybox.vert` / `skybox.frag`)

### Vertex Shader
*   **Truco Z=W**:
    ```glsl
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
    ```
    Al dividir por W, la profundidad resultante es siempre 1.0 (el plano más lejano).

### Fragment Shader
*   **Muestreo Cúbico**:
    ```glsl
    FragColor = texture(skybox, TexCoords); // TexCoords es vec3 dirección
    ```

## 4. Shader de UI/HUD (`ui.vert` / `ui.frag`)

Utilizado por `Renderer2D`.
*   **Proyección Ortográfica**: No hay perspectiva. Las coordenadas son píxeles directos (0,0 a Ancho,Alto).
*   **Color Uniforme**: Los instrumentos suelen ser monocromáticos (verde HUD), por lo que el color se pasa como uniform.

## 5. Puntos Clave para el Examen

*   **Pipeline Gráfico**: Vertex Shader -> Rasterizer -> Fragment Shader.
*   **Uniforms vs Attributes**:
    *   *Attributes*: Datos por vértice (posición, UV).
    *   *Uniforms*: Datos constantes para todo el objeto (matrices, tiempo, texturas).
*   **Model-View-Projection (MVP)**: La multiplicación estándar de matrices para llevar un vértice del espacio local a la pantalla.
