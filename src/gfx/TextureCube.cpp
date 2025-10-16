#include "TextureCube.h"
#include "GLCheck.h"
#include <iostream>
#include <array>
#include <stb/stb_image.h>

namespace gfx {

bool TextureCube::loadFromAtlas(const std::string& path, bool flipY) {
    int W = 0, H = 0;
    std::vector<unsigned char> rgba;
    
    if (!util::atlasLoadRGBA(path, W, H, rgba, flipY)) {
        std::cerr << "Failed to load atlas: " << path << std::endl;
        return false;
    }

    int S = 0;
    util::CubeLayout L;
    if (!util::atlasDetect(W, H, S, L)) {
        std::cerr << "Atlas layout not recognized (expected 4x3, 3x4, 6x1, 1x6, or 512x512): " << W << "x" << H << std::endl;
        return false;
    }

    std::cout << "Detected atlas layout: " << W << "x" << H << ", face size: " << S << "x" << S << std::endl;

    try {
        util::CubeFaces faces = util::atlasSliceToCube(rgba, W, H, S, L);
        return loadCubeFaces(faces);
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing atlas: " << e.what() << std::endl;
        return false;
    }
}

bool TextureCube::loadFromFiles(const std::array<std::string, 6>& paths, bool flipY) {
    util::CubeFaces faces;
    
    for (int i = 0; i < 6; ++i) {
        int W, H;
        if (!util::atlasLoadRGBA(paths[i], W, H, faces.face[i].pixels, flipY)) {
            std::cerr << "Failed to load cube face: " << paths[i] << std::endl;
            return false;
        }
        faces.face[i].w = W;
        faces.face[i].h = H;
        
        if (i == 0) {
            faces.size = W; // Asumimos caras cuadradas
        } else if (W != faces.size || H != faces.size) {
            std::cerr << "All cube faces must be the same size and square" << std::endl;
            return false;
        }
    }
    
    return loadCubeFaces(faces);
}

bool TextureCube::loadCubeFaces(const util::CubeFaces& faces) {
    if (!id_) {
        glGenTextures(1, &id_);
    }
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, id_);
    
    // Configurar parámetros
    setupParameters();
    
    // Targets para cada cara del cubemap en orden OpenGL
    GLenum targets[6] = {
        GL_TEXTURE_CUBE_MAP_POSITIVE_X, // +X (Right)
        GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // -X (Left)
        GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // +Y (Top)
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // -Y (Bottom)
        GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // +Z (Front)
        GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // -Z (Back)
    };
    
    // Subir cada cara
    for (int i = 0; i < 6; ++i) {
        const auto& face = faces.face[i];
        if (face.pixels.empty()) {
            std::cerr << "Empty face data for face " << i << std::endl;
            return false;
        }
        
        // Usar GL_RGBA8 en lugar de GL_SRGB8_ALPHA8 para compatibilidad
        glTexImage2D(
            targets[i], 0,           // target, level
            GL_RGBA8,                // internal format
            face.w, face.h, 0,       // width, height, border
            GL_RGBA, GL_UNSIGNED_BYTE, // format, type
            face.pixels.data()       // data
        );
        
        checkGLError(("Loading cube face " + std::to_string(i)).c_str());
    }
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    return true;
}

void TextureCube::setupParameters() {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

} // namespace gfx
