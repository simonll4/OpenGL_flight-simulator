#version 330 core

in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

uniform bool uUseTexture;
uniform sampler2D uTexture;

void main() {
    vec4 color = vColor;
    if (uUseTexture) {
        float coverage = texture(uTexture, vTexCoord).r;
        // Aplicar suavizado de bordes para texto más nítido
        // Usar smoothstep para anti-aliasing en los bordes
        float smoothedCoverage = smoothstep(0.0, 1.0, coverage);
        color = vec4(color.rgb, color.a * smoothedCoverage);
    }
    FragColor = color;
}
