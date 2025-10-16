#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "TextureCube.h"

namespace gfx {

class SkyboxRenderer {
public:
    SkyboxRenderer() = default;
    ~SkyboxRenderer();

    // No permitir copia, solo movimiento
    SkyboxRenderer(const SkyboxRenderer&) = delete;
    SkyboxRenderer& operator=(const SkyboxRenderer&) = delete;
    SkyboxRenderer(SkyboxRenderer&& other) noexcept 
        : vao_(other.vao_), vbo_(other.vbo_), shader_(std::move(other.shader_)), cube_(other.cube_) {
        other.vao_ = other.vbo_ = 0;
        other.cube_ = nullptr;
    }

    void init();                              // crea VAO/VBO, shader
    void setCubemap(TextureCube* tex) { cube_ = tex; }
    void draw(const glm::mat4& view, const glm::mat4& proj);

private:
    GLuint vao_ = 0, vbo_ = 0;
    Shader shader_;
    TextureCube* cube_ = nullptr;
    
    void createCubeGeometry();
};

} // namespace gfx
