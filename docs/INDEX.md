# Índice de Documentación - Flight Simulator HUD

Bienvenido al centro de documentación del proyecto Flight Simulator HUD. Todos los documentos están organizados por módulo para facilitar la navegación.

---

## 📚 Guía Rápida

| Necesito... | Ver Documento |
|-------------|---------------|
| Empezar con el proyecto | [README principal](../README.md) |
| Entender el módulo HUD | [Arquitectura HUD](hud/README_MODULE.md) |
| Agregar un instrumento | [Guía de Instrumentos](hud/README_INSTRUMENTS.md) |
| Usar navegación por waypoints | [Manual Waypoint Indicator](waypoints/WAYPOINT_INDICATOR_README.md) |
| Sistema gráfico | [Módulo GFX](gfx/README.md) |
| Migrar código | [Guía de Migración](hud/MIGRATION_GUIDE.md) |

---

## 📂 Documentación por Módulo

### 🖥️ Módulo HUD (Heads-Up Display)

#### [README_MODULE.md](hud/README_MODULE.md)
**Descripción**: Arquitectura completa del módulo HUD  
**Contenido**:
- Visión general del sistema
- Estructura de directorios organizada
- Principios de diseño (polimorfismo, escalabilidad)
- Guía paso a paso para agregar instrumentos
- Convenciones de código
- Rutas de includes
- Testing y validación
- Instrumentos futuros planificados

**Cuándo leer**: Al desarrollar cualquier funcionalidad HUD

---

#### [README_INSTRUMENTS.md](hud/README_INSTRUMENTS.md)
**Descripción**: Guía detallada de instrumentos implementados  
**Contenido**:
- Lista de instrumentos disponibles
- Características de cada instrumento
- Uso y configuración
- Ejemplos de código
- Troubleshooting

**Cuándo leer**: Al usar o modificar instrumentos específicos

---

#### [MIGRATION_GUIDE.md](hud/MIGRATION_GUIDE.md)
**Descripción**: Guía de migración de estructura plana a organizada  
**Contenido**:
- Estructura antes vs después
- Mapa de archivos movidos
- Cambios en includes
- Checklist de migración
- FAQ sobre la reorganización

**Cuándo leer**: Si tienes código que usa la estructura antigua del HUD

---

### 🧭 Sistema de Navegación por Waypoints

#### [WAYPOINT_INDICATOR_README.md](waypoints/WAYPOINT_INDICATOR_README.md)
**Descripción**: Manual completo del indicador de waypoints  
**Contenido**:
- Diseño del instrumento profesional
- Panel izquierdo: Brújula circular
- Panel derecho: Información digital (DIST, TURN, BRG)
- Interpretación visual paso a paso
- 4 escenarios de vuelo con ejemplos
- Técnicas de navegación
- FAQ y troubleshooting
- Referencias a sistemas reales (HSI, CDI)

**Cuándo leer**: Al usar el sistema de navegación por waypoints

**Páginas**: ~350 líneas | **Nivel**: Usuario/Desarrollador

---

#### [WAYPOINT_NAVIGATION.md](waypoints/WAYPOINT_NAVIGATION.md)
**Descripción**: Sistema completo de navegación  
**Contenido**:
- Componentes implementados (HUD + 3D + FlightData)
- Integración en main.cpp (código completo)
- Cálculo de bearing y distancia
- Captura automática de waypoints
- Sistema de coordenadas
- Personalización (colores, geometría, radio)
- Arquitectura del sistema
- Controles del teclado
- Mejoras recientes (v2.0)

**Cuándo leer**: Al implementar o modificar el sistema de waypoints

**Páginas**: ~350 líneas | **Nivel**: Desarrollador

---

### 🎨 Sistema Gráfico (GFX)

#### [README.md](gfx/README.md)
**Descripción**: Documentación del módulo gráfico  
**Contenido**:
- Sistema de renderizado 2D y 3D
- Shaders y pipelines
- Carga de modelos y texturas
- Sistema de terreno
- Skybox
- Optimizaciones

**Cuándo leer**: Al trabajar con renderizado, modelos o efectos visuales

---

## 🗺️ Mapa de Documentación

### Nivel 1: Introducción
```
README.md (raíz)
├── Descripción del proyecto
├── Características principales
├── Quick start
├── Controles básicos
└── Enlaces a documentación detallada
```

### Nivel 2: Módulos
```
docs/
├── hud/                 # Todo sobre el HUD
│   ├── Arquitectura
│   ├── Instrumentos
│   └── Migración
│
├── waypoints/           # Sistema de navegación
│   ├── Manual del indicador
│   └── Sistema completo
│
└── gfx/                 # Sistema gráfico
    └── Renderizado y recursos
```

### Nivel 3: Código
```
Comentarios en archivos .h/.cpp
├── Doxygen-style headers
├── Explicaciones inline
└── Ejemplos de uso
```

---

## 📖 Guías por Tarea

### Quiero empezar a desarrollar

1. [README principal](../README.md) - Compilación y estructura
2. [Módulo HUD](hud/README_MODULE.md) - Arquitectura
3. Explorar código en `src/`

### Quiero agregar un instrumento HUD

1. [Arquitectura HUD](hud/README_MODULE.md#cómo-agregar-un-nuevo-instrumento)
2. [Guía de Instrumentos](hud/README_INSTRUMENTS.md)
3. Ver ejemplo: `src/hud/instruments/flight/Altimeter.cpp`

### Quiero entender el sistema de waypoints

1. [Manual del Indicador](waypoints/WAYPOINT_INDICATOR_README.md) - Uso
2. [Sistema Completo](waypoints/WAYPOINT_NAVIGATION.md) - Implementación
3. Ver código: `src/hud/instruments/navigation/WaypointIndicator.cpp`

### Quiero trabajar con gráficos 3D

1. [Módulo GFX](gfx/README.md)
2. Explorar shaders en `shaders/`
3. Ver ejemplo: `src/gfx/terrain/TerrainRenderer.cpp`

### Tengo código antiguo que no compila

1. [Guía de Migración](hud/MIGRATION_GUIDE.md)
2. Actualizar includes según mapa de archivos
3. Recompilar con `make clean && make`

---

## 📊 Estadísticas de Documentación

| Módulo | Archivos | Páginas Aprox | Nivel |
|--------|----------|---------------|-------|
| **HUD** | 3 docs | ~500 líneas | Intermedio |
| **Waypoints** | 2 docs | ~700 líneas | Usuario + Dev |
| **GFX** | 1 doc | ~200 líneas | Avanzado |
| **General** | README + INDEX | ~400 líneas | Principiante |
| **TOTAL** | **7 documentos** | **~1800 líneas** | Todos |

---

## 🎯 Documentación por Audiencia

### Para Usuarios (Pilotos)
- [README principal](../README.md) - Controles y uso básico
- [Manual Waypoint Indicator](waypoints/WAYPOINT_INDICATOR_README.md) - Navegación

### Para Desarrolladores
- [Arquitectura HUD](hud/README_MODULE.md) - Sistema modular
- [Sistema de Navegación](waypoints/WAYPOINT_NAVIGATION.md) - Integración
- [Módulo GFX](gfx/README.md) - Renderizado

### Para Mantenedores
- [Guía de Migración](hud/MIGRATION_GUIDE.md) - Cambios estructurales
- [Guía de Instrumentos](hud/README_INSTRUMENTS.md) - Lista completa
- Código comentado en `src/`

---

## 🔍 Búsqueda Rápida

### Buscar por Palabra Clave

#### "Waypoint"
- [Manual del Indicador](waypoints/WAYPOINT_INDICATOR_README.md)
- [Sistema de Navegación](waypoints/WAYPOINT_NAVIGATION.md)
- [Arquitectura HUD](hud/README_MODULE.md) - Instrumentos/navigation

#### "Instrumento"
- [Arquitectura HUD](hud/README_MODULE.md)
- [Guía de Instrumentos](hud/README_INSTRUMENTS.md)

#### "Altímetro" o "SpeedIndicator"
- [Guía de Instrumentos](hud/README_INSTRUMENTS.md)
- Código: `src/hud/instruments/flight/`

#### "Compilar" o "Makefile"
- [README principal](../README.md#compilación)

#### "Controles" o "Teclado"
- [README principal](../README.md#controles)
- [Sistema de Navegación](waypoints/WAYPOINT_NAVIGATION.md#controles-del-sistema)

#### "OpenGL" o "Shader"
- [Módulo GFX](gfx/README.md)

---

## 📝 Formatos y Convenciones

### Nomenclatura de Archivos
- `README_*.md` - Documentación general de módulo
- `*_GUIDE.md` - Guías paso a paso
- `INDEX.md` - Índices y navegación
- `README.md` - Documento principal

### Estructura de Documentos
Todos los documentos siguen una estructura similar:
1. **Título y descripción**
2. **Tabla de contenidos** (si es largo)
3. **Contenido principal** con secciones
4. **Ejemplos prácticos**
5. **Referencias y recursos**

### Estilo de Escritura
- **Lenguaje**: Español técnico claro
- **Código**: Bloques con sintaxis highlighting
- **Diagramas**: ASCII art o tablas
- **Enlaces**: Relativos desde ubicación actual

---

## 🔄 Actualización de Documentación

### Última Actualización
**Fecha**: 2025  
**Versión**: 2.0  

### Cambios Recientes
- ✅ Centralización en `/docs`
- ✅ Reorganización por módulos
- ✅ Índice maestro creado
- ✅ README principal completo
- ✅ Documentación waypoints extendida

### Próximas Actualizaciones
- [ ] Documentación de AttitudeIndicator
- [ ] Guía de shaders
- [ ] Tutoriales en video
- [ ] Diagramas de arquitectura visuales

---

## 💡 Consejos de Navegación

### Atajos Útiles

**Desde la raíz del proyecto**:
```bash
# Ver README principal
cat README.md

# Ver índice de docs
cat docs/INDEX.md

# Buscar en documentación
grep -r "término" docs/

# Listar toda la documentación
find docs -name "*.md"
```

**Desde VS Code / Editor**:
- Usar búsqueda global (Ctrl+Shift+F)
- Navegar con Ctrl+Click en enlaces
- Vista previa de Markdown (Ctrl+Shift+V)

---

## 📧 Contacto y Soporte

### Documentación Faltante o Incorrecta
Si encuentras:
- Enlaces rotos
- Documentación desactualizada
- Información faltante
- Errores técnicos

Por favor reporta en el sistema de issues del proyecto.

### Contribuir a la Documentación
Pull requests bienvenidos para:
- Correcciones
- Clarificaciones
- Traducciones
- Nuevos ejemplos
- Diagramas

---

## 📚 Recursos Externos

### Aprendizaje
- [LearnOpenGL.com](https://learnopengl.com/) - Tutoriales OpenGL
- [OpenGL Wiki](https://www.khronos.org/opengl/wiki/)
- [GLM Documentation](https://glm.g-truc.net/)

### Aviación
- FAA Instrument Flying Handbook
- ICAO Annex 6 - Cockpit Standards
- HSI Operation Manuals

### C++ y Desarrollo
- [C++ Reference](https://en.cppreference.com/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Git Handbook](https://guides.github.com/)

---

**¡Feliz lectura y desarrollo!** 📖✨
