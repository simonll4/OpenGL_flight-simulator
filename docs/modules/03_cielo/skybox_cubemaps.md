# Análisis de Cielo: Skybox y Cubemaps

## 1. Concepto de Skybox

Un Skybox es una técnica para crear la ilusión de un entorno lejano e infinito. Se renderiza un cubo grande alrededor de la cámara, texturizado con imágenes que representan el cielo, montañas lejanas, etc.

## 2. Implementación: `SkyboxRenderer`

### Geometría
Se utiliza un **Cubo Unitario** (de -1 a 1 en cada eje). No es necesario que sea gigante, ya que el truco está en cómo se renderiza (sin traslación).

### Matriz de Vista (El Truco)
Para que el cielo parezca estar infinitamente lejos, no debe moverse cuando el avión se desplaza (traslación), pero sí debe rotar cuando el avión gira.

```cpp
// Se toma la submatriz 3x3 (rotación) y se convierte de nuevo a 4x4
glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
```

### Depth Testing (`GL_LEQUAL`)
El skybox se suele renderizar al final o con una configuración especial de profundidad.
En este proyecto:
1.  Se usa `glDepthFunc(GL_LEQUAL)`.
2.  En el vertex shader, se fuerza la posición Z a ser `w` (lo que resulta en profundidad 1.0, el máximo).
    ```glsl
    vec4 pos = uProj * uView * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // Z = W, tras la división perspectiva Z/W = 1.0
    ```
Esto asegura que el skybox se dibuje "detrás" de todo lo demás.

## 3. Texturas: Cubemaps (`TextureCube`)

Un **Cubemap** es un tipo especial de textura en OpenGL (`GL_TEXTURE_CUBE_MAP`) que contiene 6 caras:
*   `GL_TEXTURE_CUBE_MAP_POSITIVE_X` (Right)
*   `GL_TEXTURE_CUBE_MAP_NEGATIVE_X` (Left)
*   `GL_TEXTURE_CUBE_MAP_POSITIVE_Y` (Top)
*   `GL_TEXTURE_CUBE_MAP_NEGATIVE_Y` (Bottom)
*   `GL_TEXTURE_CUBE_MAP_POSITIVE_Z` (Front)
*   `GL_TEXTURE_CUBE_MAP_NEGATIVE_Z` (Back)

El fragment shader utiliza un vector de dirección 3D (`vec3`) para muestrear la textura, en lugar de coordenadas UV 2D.

```glsl
uniform samplerCube uCube;
in vec3 TexCoords;
// ...
FragColor = texture(uCube, TexCoords);
```

## 4. Puntos Clave para el Examen

*   **Invariancia a la Traslación**: Por qué eliminamos la parte de traslación de la matriz View.
*   **SamplerCube**: Cómo se accede a los texels usando un vector dirección.
*   **Optimización**: Dibujar el skybox con Z=1.0 permite que el *Early Z-Test* descarte los píxeles del cielo que están ocultos por el terreno o el avión (si se dibuja al final).
