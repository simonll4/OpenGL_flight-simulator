# Estructura de Assets

El directorio `assets/` contiene todos los recursos binarios y de configuración necesarios para la ejecución del simulador.

## Estructura de Directorios

```
assets/
├── fonts/          # Fuentes TrueType para la interfaz
├── missions/       # Definiciones de misiones en formato JSON
├── models/         # Modelos 3D (avión, objetos del mundo)
└── textures/       # Texturas para modelos, terreno y skybox
```

## Detalles por Categoría

### 1. Fuentes (`fonts/`)
Contiene archivos `.ttf` utilizados por el renderizador de texto `gfx::TrueTypeFont`.
- **Uso**: Interfaz del Mission Planner, Overlays y textos de alta calidad.
- **Fuente principal**: Se recomienda usar fuentes monoespaciadas o legibles para instrumentos (ej. Roboto Mono).

### 2. Misiones (`missions/`)
- **Archivo principal**: `missions.json`
- **Formato**: JSON array conteniendo objetos de definición de misión.
- **Campos clave**:
  - `id`: Identificador único.
  - `name`, `description`: Textos descriptivos.
  - `startPosition`: Coordenadas `{x, y, z}` de inicio.
  - `waypoints`: Lista de puntos de navegación.
  - `environment`: Configuración de clima y hora.

### 3. Modelos (`models/`)
Modelos 3D cargados mediante Assimp.
- **Formatos soportados**: `.obj`, `.fbx`, `.dae`, etc.
- **Modelo principal**: F-16 o aeronave equivalente para la vista en tercera persona.

### 4. Texturas (`textures/`)
Recursos gráficos para materiales y entorno.
- **Terreno**: Texturas de color (albedo) usadas en el plano texturizado (`assets/textures/terrain/texture.png`).
- **Skybox**: 6 imágenes para el mapa de cubo (formato usualmente `right`, `left`, `top`, `bottom`, `front`, `back`).
- **UI**: Iconos o elementos gráficos 2D si los hubiera.

## Gestión de Recursos
La clase `core::Application` y los distintos renderizadores (`gfx::*`) son responsables de cargar estos assets al inicio.
- Las rutas suelen ser relativas al ejecutable o definidas en constantes globales.
- Se recomienda mantener los nombres de archivo en minúsculas y sin espacios para compatibilidad multiplataforma.
