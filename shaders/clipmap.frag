#version 330 core

out vec4 FragColor;

// Uniforms
uniform vec3 u_Background;
uniform vec3 u_CameraPos;
uniform float u_FogMinDist;
uniform float u_FogMaxDist;
uniform bool u_ShowLODColors;

// Texturas
uniform sampler2D u_Heightmap;
uniform sampler2D u_Normalmap;
uniform sampler2D u_Texture;

// Inputs desde vertex shader
in vec3 v_Color;
in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoord;
in float v_LODLevel;

/**
 * @brief Calcula iluminación direccional simple
 */
vec3 calculateDirLight(vec3 lightDir, vec3 normal, vec3 baseColor)
{
    float ka = 0.6;  // Ambient
    float kd = 0.8;  // Diffuse

    vec3 lightColor = vec3(1.0);

    // Ambient
    vec3 ambient = ka * lightColor;
    
    // Diffuse (Lambert)
    float diff = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    vec3 diffuse = kd * diff * lightColor;

    return (ambient + diffuse) * baseColor;
}

/**
 * @brief Calcula factor de niebla
 */
float calculateFog(float dist, float fogMin, float fogMax)
{
    float fogFactor = (fogMax - dist) / (fogMax - fogMin);
    return clamp(fogFactor, 0.0, 1.0);
}

void main()
{
    // Dirección de la luz (sol)
    vec3 lightDir = normalize(vec3(-2.0, 4.0, -1.0));

    // Color del terreno (con tiling para detalle)
    vec3 terrainColor = texture(u_Texture, v_TexCoord * 10.0).rgb;
    
    // Si queremos ver colores de LOD, mezclar
    if (u_ShowLODColors) {
        terrainColor = mix(terrainColor, v_Color, 0.3);
    }

    // Aplicar iluminación
    vec3 litColor = calculateDirLight(lightDir, v_Normal, terrainColor);

    // Calcular fog
    float dist = length(v_FragPos - u_CameraPos);
    float fogFactor = calculateFog(dist, u_FogMinDist, u_FogMaxDist);

    vec4 fogColor = vec4(u_Background, 1.0);
    vec4 finalColor = vec4(litColor, 1.0);

    FragColor = mix(fogColor, finalColor, fogFactor);
}
