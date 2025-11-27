# Sistema de Cámaras

## 1. Clase `CameraRig`

La clase `systems::CameraRig` encapsula toda la lógica de visualización. Calcula dos matrices fundamentales para el pipeline gráfico:

### Matriz de Vista (View Matrix)
Define la posición y orientación de la cámara en el mundo. Se calcula usando `glm::lookAt`:
```cpp
viewMatrix_ = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);
```

### Matriz de Proyección (Projection Matrix)
Define cómo se proyecta el mundo 3D en la pantalla 2D. Se usa una proyección perspectiva:
```cpp
glm::perspective(fov, aspectRatio, nearPlane, farPlane);
```

## 2. Modos de Cámara

### Primera Persona (Cockpit)
La cámara se "pega" a la posición del avión con un pequeño offset (la cabeza del piloto) y copia exactamente su orientación.
*   **Sensación**: Inmersiva, transmite cada rotación.

### Tercera Persona (Chase)
La cámara sigue al avión desde atrás y arriba.
*   **Seguimiento**: Actualmente es rígido (sin lerp). Se calcula la posición objetivo detrás/arriba y se asigna directo. Si se quisiera el “lag”, habría que interpolar `cameraPos_`/`cameraFront_` hacia un objetivo con un factor dependiente de `dt` y/o velocidad.

### Cámara Cinematográfica
Alterna entre puntos fijos en el mundo (torres de control, montañas) y sigue al avión con la mirada (`lookAt`).
*   **Generación Dinámica**: Si el avión se aleja demasiado de todos los puntos conocidos (> 300m), el sistema genera automáticamente nuevos puntos de cámara en posiciones aleatorias alrededor del avión para mantener la acción visible.
*   **Histéresis**: Para evitar cambios de cámara frenéticos, el sistema solo cambia de cámara si la nueva posición es significativamente mejor (más cercana) o si la actual ya no es válida.

## 3. Far Plane Dinámico

Un detalle técnico importante es el ajuste dinámico del plano lejano (*Far Plane*).
*   **A baja altura**: El horizonte está cerca (oculto por montañas). `farPlane` puede ser menor para ganar precisión en el Z-Buffer.
*   **A gran altura**: Se puede ver muy lejos (cientos de km). `farPlane` debe aumentar drásticamente (ej. 100km o más) para no cortar el terreno.

```cpp
dynamicFarPlane_ = baseFar + (altitude * factor);
```

## 4. Puntos Clave para el Examen

*   **LookAt**: Entender los vectores `eye`, `center`, `up`.
*   **FOV (Field of View)**: Ángulo de visión vertical. Un FOV alto da sensación de velocidad pero distorsiona los bordes.
*   **Z-Fighting**: Problema que ocurre si el rango `near` - `far` es demasiado grande con precisión limitada (aunque con Logarithmic Depth Buffer o Reverse Z se mitiga, aquí usamos ajuste dinámico).
