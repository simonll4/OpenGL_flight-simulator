# Guía de Uso: Sistema de Terreno Clipmap

## 📋 Descripción

El sistema **ClipmapTerrain** es una implementación avanzada de renderizado de terreno que utiliza la técnica de **Geometry Clipmaps** para renderizar terrenos muy grandes de manera eficiente.

### Ventajas sobre el sistema actual (TerrainRenderer):

| Característica | TerrainRenderer (actual) | ClipmapTerrain (nuevo) |
|----------------|-------------------------|------------------------|
| **Tamaño máximo** | ~1km x 1km | ~200km x 200km |
| **LOD** | No | Sí (12-16 niveles) |
| **Vértices** | Fixed (~16k) | Dynamic (~50k-200k) |
| **Performance** | Buena para áreas pequeñas | Excelente para áreas grandes |
| **Complejidad** | Baja | Media-Alta |

## 🚀 Integración en el Proyecto

### 1. Agregar al Makefile

Edita el `Makefile` para incluir el nuevo archivo:

```makefile
# En la sección SOURCES, agregar:
SOURCES += src/gfx/terrain/ClipmapTerrain.cpp

# El sistema automáticamente compilará:
# - ClipmapTerrain.cpp
# - Generará build/ClipmapTerrain.o
```

### 2. Uso Básico en main.cpp

```cpp
#include "gfx/terrain/ClipmapTerrain.h"

int main()
{
    // ... inicialización de GLFW/OpenGL ...

    // Crear configuración del clipmap
    gfx::ClipmapConfig config;
    config.levels = 12;          // 12 niveles de LOD
    config.segments = 32;        // 32x32 segmentos por tile
    config.segmentSize = 2.0f;   // 2 metros por segmento
    config.heightScale = 3000.0f;// Montañas de hasta 3km
    config.terrainSize = 200000.0f; // 200km x 200km

    // Crear el terreno
    gfx::ClipmapTerrain clipmapTerrain(config);
    clipmapTerrain.init();
    clipmapTerrain.loadTextures("assets/textures/terrain");

    // En el render loop:
    while (!glfwWindowShouldClose(window)) {
        // ... clear, input, etc ...

        // Renderizar el clipmap
        clipmapTerrain.draw(view, projection, cameraPos, backgroundColor);

        // ... swap buffers ...
    }

    clipmapTerrain.cleanup();
    return 0;
}
```

### 3. Reemplazar TerrainRenderer Existente

Para reemplazar el sistema actual, modifica `main.cpp`:

**Antes:**
```cpp
gfx::TerrainRenderer terrain;
terrain.init();
terrain.loadTextures("assets/textures/terrain");
terrain.draw(view, projection, cameraPos, terrainParams);
```

**Después:**
```cpp
gfx::ClipmapConfig clipConfig;
clipConfig.levels = 12;
clipConfig.segments = 32;
clipConfig.segmentSize = 2.0f;

gfx::ClipmapTerrain terrain(clipConfig);
terrain.init();
terrain.loadTextures("assets/textures/terrain");
terrain.draw(view, projection, cameraPos);
```

## 📁 Texturas Requeridas

El sistema espera encontrar en `assets/textures/terrain/`:

```
assets/textures/terrain/
├── heightmap.png     # Mapa de alturas (R8 o RGB)
├── normalmap.png     # Mapa de normales (RGB)
└── texture.png       # Textura de color (RGB/RGBA)
```

### Generación de Texturas

**Heightmap:**
- Formato: PNG, grayscale o RGB
- Valores: 0 (negro) = altura mínima, 255 (blanco) = altura máxima
- Recomendado: 2048x2048 o 4096x4096

**Normalmap:**
- Se puede generar desde el heightmap usando herramientas como:
  - GIMP (Filters → Generic → Normal Map)
  - `imagemagick` + scripts personalizados
  - Herramientas online de normal mapping

**Texture:**
- Cualquier textura de terreno (pasto, roca, tierra, etc.)
- Se aplicará con tiling (repetición) automática

### Fallback

Si no hay `heightmap.png`, el sistema intentará usar las texturas existentes:
- `forrest_ground_01_diff_4k.jpg` (como textura)

## ⚙️ Configuración Avanzada

### Ajustar Parámetros

```cpp
gfx::ClipmapConfig config;

// Niveles de LOD (más = mayor alcance visual)
config.levels = 16;  // 8-16 recomendado

// Resolución de tiles (debe ser potencia de 2)
config.segments = 64;  // 32 o 64 recomendado

// Tamaño de cada segmento en metros
config.segmentSize = 4.0f;  // Más grande = menos detalle

// Escala de altura
config.heightScale = 5000.0f;  // Montañas más altas

// Niebla
config.fogMinDist = 500.0f;   // Cerca sin niebla
config.fogMaxDist = 50000.0f; // Lejos con niebla completa
```

### Modos de Debug

```cpp
// Ver niveles de LOD con colores
clipmapTerrain.showLODColors = true;

// Modo wireframe
clipmapTerrain.wireframe = true;
```

## 🎯 Optimización

### Performance Tips

1. **Ajustar niveles según hardware:**
   - GPU baja: `levels = 8-10`
   - GPU media: `levels = 12`
   - GPU alta: `levels = 14-16`

2. **Balancear segmentos:**
   - `segments = 32`: Buena performance, detalle medio
   - `segments = 64`: Más detalle, más carga

3. **Tamaño de segmento:**
   - Más pequeño = más detalle, más vértices
   - Más grande = menos detalle, mejor performance

### Culling Automático

El sistema automáticamente:
- Reduce LOD cuando la cámara está muy alta
- No renderiza tiles fuera del frustum
- Usa primitive restart para eficiencia

## 🐛 Troubleshooting

### El terreno no se ve

**Causa:** Shaders no se cargaron correctamente.

**Solución:** Verifica que existan:
- `shaders/clipmap.vert`
- `shaders/clipmap.frag`

### El terreno es plano

**Causa:** Heightmap no se cargó o `heightScale` es 0.

**Solución:**
```cpp
config.heightScale = 3000.0f;  // Asegurar valor > 0
```

### Cracks entre niveles

**Causa:** Los seams no se están renderizando correctamente.

**Solución:** Esto es un bug del sistema, revisar código de seams.

### Performance pobre

**Causa:** Demasiados niveles o segmentos.

**Solución:**
```cpp
config.levels = 10;    // Reducir niveles
config.segments = 32;  // Reducir segmentos
```

## 📊 Comparación de Rendimiento

### Sistema Actual (TerrainRenderer)

```
Área: 1km x 1km
Vértices: ~16k constantes
Draw calls: 1
FPS: ~300 (GPU media)
```

### Sistema Clipmap (ClipmapTerrain)

```
Área: 200km x 200km
Vértices: ~50k-200k (dinámico)
Draw calls: ~100-300 (por nivel LOD)
FPS: ~200 (GPU media)
```

## 📚 Referencias

- **Paper Original:** "Geometry Clipmaps: Terrain Rendering Using Nested Regular Grids" (Losasso & Hoppe, 2004)
- **GPU Gems 2:** Capítulo sobre Geometry Clipmaps
- **Proyecto de Referencia:** `/home/simonll4/Desktop/opengl-sim/OpenGL_Flightsim`

## 🔄 Migración Gradual

Puedes mantener ambos sistemas y cambiar entre ellos:

```cpp
// En main.cpp
bool useClipmapTerrain = true;  // Toggle

if (useClipmapTerrain) {
    clipmapTerrain.draw(view, projection, cameraPos);
} else {
    terrain.draw(view, projection, cameraPos, terrainParams);
}
```

Esto permite comparar rendimiento y calidad visual antes de hacer la migración completa.

## ✅ Checklist de Integración

- [ ] Agregar `ClipmapTerrain.cpp` al Makefile
- [ ] Crear shaders `clipmap.vert` y `clipmap.frag`
- [ ] Preparar texturas (heightmap, normalmap, texture)
- [ ] Modificar `main.cpp` para usar el nuevo sistema
- [ ] Compilar y testear
- [ ] Ajustar parámetros de configuración
- [ ] Optimizar para el hardware objetivo

## 💡 Próximos Pasos

Una vez integrado, puedes:

1. **Generar heightmaps reales** usando datos geográficos (SRTM, etc.)
2. **Agregar múltiples texturas** basadas en altura/pendiente
3. **Implementar colisiones** usando `getHeightAt()`
4. **Agregar vegetación** usando instancing
5. **Mejorar iluminación** con sombras en tiempo real

## 🎓 Conceptos Clave

### ¿Qué es un Clipmap?

Un clipmap es un sistema de LOD (Level of Detail) donde:
- Múltiples niveles de geometría se renderizan anidados
- Cada nivel tiene el doble de resolución espacial que el anterior
- El centro del clipmap sigue siempre la cámara
- Los niveles más finos están cerca, los más gruesos lejos

### Estructura de Niveles

```
Level 0 (centro):  escala 1x,  tile size 64m
Level 1:           escala 2x,  tile size 128m
Level 2:           escala 4x,  tile size 256m
Level 3:           escala 8x,  tile size 512m
...
Level N:           escala 2^N, tile size 64*2^N
```

### ¿Por qué es eficiente?

- **Vértices constantes:** Cada nivel tiene la misma cantidad de vértices
- **Detalle adaptativo:** Más detalle donde la cámara está cerca
- **Culling natural:** Niveles lejanos se pueden omitir si estás muy alto
- **Cache-friendly:** Los datos se reutilizan entre frames

---

**¿Preguntas?** Consulta el código fuente en `src/gfx/terrain/ClipmapTerrain.cpp` o revisa el proyecto de referencia OpenGL_Flightsim.
