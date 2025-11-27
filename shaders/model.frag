#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Material parameters
uniform sampler2D texture_diffuse1;

// Flags to check if textures are available
uniform bool hasTextureDiffuse;

// Light properties
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main() {
    // Get material color
    vec3 objectColor = hasTextureDiffuse ? texture(texture_diffuse1, TexCoords).rgb : vec3(0.7, 0.7, 0.7);
    
    // Normalize the normal (interpolation can denormalize it)
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);
    
    // Ambient
    float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * objectColor;
    
    // Diffuse (Lambertian)
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * objectColor;
    
    // Specular (Blinn-Phong)
    float specularStrength = 0.5;
    float spec = pow(max(dot(N, H), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;
    
    // Combine
    vec3 result = ambient + diffuse + specular;
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
