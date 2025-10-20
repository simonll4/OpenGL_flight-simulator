# Módulo GFX - Sistema de Renderizado

Estructura organizada del sistema gráfico del simulador de vuelo.

## Estructura de Directorios

```
gfx/
├── core/           # Componentes fundamentales de OpenGL
├── geometry/       # Geometría y modelos 3D
├── rendering/      # Renderizado 2D (HUD)
├── terrain/        # Sistema de terreno
└── skybox/         # Sistema de cielo/skybox
```

## Módulos

### 📦 `core/` - Núcleo OpenGL
Componentes fundamentales para trabajar con OpenGL.

- **`Shader.h/cpp`** - Sistema de shaders (compilación, linking, uniforms)
- **`GLCheck.h`** - Utilidades de debugging OpenGL (macros de error checking)

**Usado por:** Todos los demás módulos

---

### 🎨 `geometry/` - Geometría 3D
Carga y renderizado de modelos 3D.

- **`Mesh.h/cpp`** - Representación de una malla 3D (vértices, índices, texturas)
- **`Model.h/cpp`** - Carga de modelos 3D usando Assimp (GLB, OBJ, FBX, etc.)

**Usado por:** `main.cpp` (modelo F-16)

---

### 🖼️ `rendering/` - Renderizado 2D
Sistema de renderizado para el HUD y elementos 2D.

- **`Renderer2D.h/cpp`** - Renderizador batch de primitivas 2D (rectángulos, líneas)
- **`TextRenderer.h/cpp`** - Renderizado de texto con 7 segmentos (estilo digital)

**Usado por:** `hud/` (todos los instrumentos)

---

### 🏔️ `terrain/` - Sistema de Terreno
Generación y renderizado del terreno infinito.

- **`TerrainMesh.h/cpp`** - Generación de grid procedural para el terreno
- **`TerrainRenderer.h/cpp`** - Renderizado con triplanar mapping, texturas PBR y niebla

**Características:**
- Triplanar mapping (evita distorsión en superficies verticales)
- Texturas PBR (albedo, roughness)
- Sistema de floating origin (terreno infinito)
- Niebla exponencial

**Usado por:** `main.cpp`

---

### ☁️ `skybox/` - Sistema de Cielo
Renderizado del cielo envolvente.

- **`TextureCube.h/cpp`** - Carga y gestión de cubemaps (6 caras)
- **`SkyboxRenderer.h/cpp`** - Renderizado del skybox con depth trick

**Características:**
- Carga desde atlas 3x2 o 6 archivos individuales
- Renderizado optimizado (sin traslación, depth = 1.0)

**Usado por:** `main.cpp`

---

## Dependencias entre Módulos

```
core/
  └─> (usado por todos)

geometry/
  └─> core/

rendering/
  └─> core/

terrain/
  └─> core/

skybox/
  └─> core/
  └─> util/ (ImageAtlas para cargar atlas de cubemap)
```

## Convenciones de Código

### Includes
- **Dentro del mismo módulo:** `#include "Archivo.h"`
- **Otro módulo de gfx:** `#include "../modulo/Archivo.h"`
- **Fuera de gfx:** `#include "../../carpeta/Archivo.h"`

### Namespaces
Todos los archivos están en el namespace `gfx`:

```cpp
namespace gfx {
    // código
}
```

### Estilo
- **Clases:** PascalCase (`TerrainRenderer`)
- **Funciones/métodos:** camelCase (`loadTextures()`)
- **Variables privadas:** camelCase con sufijo `_` (`shader_`, `vao_`)
- **Constantes:** UPPER_SNAKE_CASE (`MAX_VERTICES`)

## Agregar Nuevo Módulo

1. Crear subdirectorio en `src/gfx/`
2. Agregar archivos `.h` y `.cpp`
3. Actualizar `Makefile`:
   - Agregar a `SRC_DIR`
   - Agregar wildcard en `CPP_SOURCES`
4. Usar includes relativos correctos
5. Documentar en este README

## Ver También

- `/src/hud/README_INSTRUMENTS.md` - Sistema de instrumentos del HUD
- `/notas.txt` - Notas técnicas sobre renderizado de texto
