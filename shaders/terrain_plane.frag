#version 330 core

out vec4 FragColor;

uniform vec3 u_Background;
uniform vec3 u_CameraPos;
uniform float u_FogMinDist;
uniform float u_FogMaxDist;
uniform sampler2D u_Texture;

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoord;

vec3 calculateDirLight(vec3 lightDir, vec3 normal, vec3 baseColor)
{
    float ka = 0.6;
    float kd = 0.8;

    vec3 lightColor = vec3(1.0);

    vec3 ambient = ka * lightColor;
    float diff = max(dot(normalize(normal), normalize(lightDir)), 0.0);
    vec3 diffuse = kd * diff * lightColor;

    return (ambient + diffuse) * baseColor;
}

float calculateFog(float dist, float fogMin, float fogMax)
{
    float fogFactor = (fogMax - dist) / (fogMax - fogMin);
    return clamp(fogFactor, 0.0, 1.0);
}

void main()
{
    vec3 lightDir = normalize(vec3(-2.0, 4.0, -1.0));

    vec3 terrainColor = texture(u_Texture, v_TexCoord).rgb;
    vec3 litColor = calculateDirLight(lightDir, v_Normal, terrainColor);

    float dist = length(v_FragPos - u_CameraPos);
    float fogFactor = calculateFog(dist, u_FogMinDist, u_FogMaxDist);

    vec4 fogColor = vec4(u_Background, 1.0);
    vec4 finalColor = vec4(litColor, 1.0);

    FragColor = mix(fogColor, finalColor, fogFactor);
}
