#version 330 core
layout (location = 0) in vec3 a_Pos;

// Matrices
uniform mat4 u_View;
uniform mat4 u_Model;
uniform mat4 u_Projection;

// Texturas
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;

// Parámetros del clipmap
uniform vec3  u_CameraPos;
uniform float u_Scale;
uniform float u_SegmentSize;
uniform float u_Level;
uniform float u_TerrainSize;
uniform float u_HeightScale;
uniform float u_HeightOffset;
uniform bool  u_ShowLODColors;

// Outputs
out vec3 v_Color;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec2 v_TexCoord;
out float v_LODLevel;

/**
 * @brief Escala un valor de un rango a otro
 */
float scaleValue(float input_val, float in_min, float in_max, float out_min, float out_max)
{
    return (input_val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Obtiene la altura desde el heightmap
 */
float getHeight(vec2 uv)
{
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        return 0.0;
    }

    float height = texture(u_Heightmap, uv).r;
    return u_HeightScale * height + u_HeightOffset;
}

/**
 * @brief Obtiene la normal desde el normalmap
 */
vec3 getNormal(vec2 uv)
{
    // Leer directamente y normalizar (igual que el shader de referencia)
    return normalize(texture(u_Normalmap, uv).rgb);
}

/**
 * @brief Convierte posición mundial a UV
 */
vec2 worldToUV(vec2 worldPos)
{
    float factor = u_TerrainSize / 2.0;
    vec2 coord = worldPos / factor;
    coord.x = scaleValue(coord.x, -1.0, 1.0, 0.0, 1.0);
    coord.y = scaleValue(coord.y, -1.0, 1.0, 0.0, 1.0);
    return coord;
}

void main()
{
    // Transformar vértice a espacio del mundo
    v_FragPos = vec3(u_Model * vec4(a_Pos, 1.0));
    
    // Calcular UV desde posición mundial
    v_TexCoord = worldToUV(v_FragPos.xz);

    // Terreno plano en Y=0 (sin elevaciones del heightmap)
    v_FragPos.y = 0.0;

    // Normal apuntando hacia arriba para terreno plano
    v_Normal = vec3(0.0, 1.0, 0.0);

    // Nivel de LOD para debug
    v_LODLevel = u_Level;

    // Color basado en LOD (para debug)
    if (u_ShowLODColors) {
        float t = u_Level;
        v_Color = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), t);
    } else {
        v_Color = vec3(1.0);
    }

    // Posición final en clip space
    gl_Position = u_Projection * u_View * vec4(v_FragPos, 1.0);
}
