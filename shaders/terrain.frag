#version 330 core
out vec4 FragColor;

in vec3 vWorldPos;
in vec3 vNormal;

uniform sampler2D uAlbedo;
uniform sampler2D uNormal;
uniform sampler2D uRough;
uniform sampler2D uDetailAlbedo;
uniform sampler2D uDetailNormal;

uniform vec3  uCamPos;
uniform vec3  uColorTint;
uniform float uTileMacro;
uniform float uTileDetail;
uniform float uDetailStr;
uniform float uFogDensity;

// Triplanar mapping weights
vec3 triplanarWeights(vec3 n) {
    vec3 an = abs(normalize(n));
    an = pow(an, vec3(4.0));
    return an / (an.x + an.y + an.z);
}

vec2 uvX(vec3 p, float s){ return p.zy * s; }
vec2 uvY(vec3 p, float s){ return p.xz * s; }
vec2 uvZ(vec3 p, float s){ return p.xy * s; }

void main() {
    vec3 N = normalize(vNormal);
    vec3 W = vWorldPos;

    vec3 w = triplanarWeights(N);

    // Macro textures
    vec3 aX = texture(uAlbedo, uvX(W, uTileMacro)).rgb;
    vec3 aY = texture(uAlbedo, uvY(W, uTileMacro)).rgb;
    vec3 aZ = texture(uAlbedo, uvZ(W, uTileMacro)).rgb;
    vec3 albedo = aX*w.x + aY*w.y + aZ*w.z;

    // Detail albedo
    vec3 daX = texture(uDetailAlbedo, uvX(W, uTileDetail)).rgb;
    vec3 daY = texture(uDetailAlbedo, uvY(W, uTileDetail)).rgb;
    vec3 daZ = texture(uDetailAlbedo, uvZ(W, uTileDetail)).rgb;
    vec3 detailA = daX*w.x + daY*w.y + daZ*w.z;

    albedo = mix(albedo, albedo * detailA, uDetailStr);
    albedo *= uColorTint;

    // Roughness
    float rX = texture(uRough, uvX(W, uTileMacro)).r;
    float rY = texture(uRough, uvY(W, uTileMacro)).r;
    float rZ = texture(uRough, uvZ(W, uTileMacro)).r;
    float rough = clamp((rX*w.x + rY*w.y + rZ*w.z), 0.04, 1.0);

    // Simple lighting (Lambert + ambient)
    vec3 L = normalize(vec3(0.3, 1.0, 0.2));
    float NdotL = max(dot(N, L), 0.0);
    vec3 color = albedo * (0.15 + 0.85 * NdotL * (1.0 - 0.5*rough));

    // Exponential fog
    float dist = length(uCamPos - W);
    float fog = 1.0 - exp(-uFogDensity * dist);
    vec3 fogColor = vec3(0.55, 0.65, 0.75);
    color = mix(color, fogColor, clamp(fog, 0.0, 1.0));

    FragColor = vec4(color, 1.0);
}
