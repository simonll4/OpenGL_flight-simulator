#pragma once
#include <string>
#include <array>

extern "C" {
#include <glad/glad.h>
}

#include "../util/ImageAtlas.h"

namespace gfx {

class TextureCube {
public:
    TextureCube() = default;
    ~TextureCube() { 
        if (id_) glDeleteTextures(1, &id_); 
    }

    // No permitir copia, solo movimiento
    TextureCube(const TextureCube&) = delete;
    TextureCube& operator=(const TextureCube&) = delete;
    TextureCube(TextureCube&& other) noexcept : id_(other.id_) { other.id_ = 0; }
    TextureCube& operator=(TextureCube&& other) noexcept {
        if (this != &other) {
            if (id_) glDeleteTextures(1, &id_);
            id_ = other.id_;
            other.id_ = 0;
        }
        return *this;
    }

    // Carga desde atlas (PNG/JPG LDR). Usa GL_SRGB8_ALPHA8 para gamma correcta.
    bool loadFromAtlas(const std::string& path, bool flipY = false);
    
    // Carga desde 6 archivos individuales
    bool loadFromFiles(const std::array<std::string, 6>& paths, bool flipY = false);

    void bind() const { glBindTexture(GL_TEXTURE_CUBE_MAP, id_); }
    void bindUnit(GLuint unit) const { 
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, id_); 
    }
    GLuint id() const { return id_; }

private:
    GLuint id_ = 0;
    
    void setupParameters();
    bool loadCubeFaces(const util::CubeFaces& faces);
};

} // namespace gfx
