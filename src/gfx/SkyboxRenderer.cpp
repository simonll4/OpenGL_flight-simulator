#include "SkyboxRenderer.h"
#include "GLCheck.h"
#include <array>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

// Geometría del cubo para skybox (36 vértices, solo posiciones)
static const float CUBE_VERTICES[108] = {
    // Cara trasera (Z-)
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    // Cara frontal (Z+)
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,

    // Cara izquierda (X-)
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    // Cara derecha (X+)
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,

    // Cara inferior (Y-)
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,

    // Cara superior (Y+)
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f
};

SkyboxRenderer::~SkyboxRenderer() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

void SkyboxRenderer::init() {
    // Crear geometría del cubo
    createCubeGeometry();
    
    // Cargar shaders
    try {
        shader_.load("shaders/skybox.vert", "shaders/skybox.frag");
        std::cout << "Skybox shaders loaded successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load skybox shaders: " << e.what() << std::endl;
        throw;
    }
}

void SkyboxRenderer::createCubeGeometry() {
    // Generar VAO y VBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    
    glBindVertexArray(vao_);
    
    // Subir datos del cubo
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);
    
    // Configurar atributos de vértice (solo posición)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    checkGLError("Creating skybox geometry");
}

void SkyboxRenderer::draw(const glm::mat4& view, const glm::mat4& proj) {
    if (!cube_) {
        std::cerr << "No cubemap texture set for skybox" << std::endl;
        return;
    }
    
    // Cambiar función de profundidad para skybox
    glDepthFunc(GL_LEQUAL);
    
    shader_.use();
    
    // Eliminar traslación de la matriz de vista (mantener solo rotación)
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    
    // Establecer uniformes
    shader_.setMat4("uView", viewNoTranslation);
    shader_.setMat4("uProj", proj);
    shader_.setInt("uCube", 0);
    
    // Bindear textura del cubemap
    cube_->bindUnit(0);
    
    // Dibujar cubo
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    // Restaurar función de profundidad
    glDepthFunc(GL_LESS);
}

} // namespace gfx
