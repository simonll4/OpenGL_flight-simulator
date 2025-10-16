#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;

uniform mat4 uViewProj;
uniform vec3 uGridOffset;   // XZ del "snap", y = groundY

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    vec3 wp = aPos + uGridOffset;
    vWorldPos = wp;
    vNormal = aNormal; // (0,1,0) para plano
    gl_Position = uViewProj * vec4(wp, 1.0);
}
