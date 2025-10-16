#include "ImageAtlas.h"
#include <iostream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace util {

bool atlasLoadRGBA(const std::string& path, int& W, int& H, std::vector<unsigned char>& rgba, bool flipY) {
    stbi_set_flip_vertically_on_load(flipY);
    
    int channels;
    unsigned char* data = stbi_load(path.c_str(), &W, &H, &channels, 4); // Force RGBA
    
    if (!data) {
        std::cerr << "Failed to load image: " << path << std::endl;
        return false;
    }
    
    rgba.assign(data, data + W * H * 4);
    stbi_image_free(data);
    
    return true;
}

bool atlasDetect(int W, int H, int& S, CubeLayout& L) {
    // 4x3 horizontal cross (2048x1536)
    if (W * 3 == H * 4 && W % 4 == 0 && H % 3 == 0) {
        S = W / 4;
        L = CubeLayout::HORIZONTAL_CROSS_4x3;
        return true;
    }
    
    // 3x4 vertical cross
    if (H * 3 == W * 4 && W % 3 == 0 && H % 4 == 0) {
        S = W / 3;
        L = CubeLayout::VERTICAL_CROSS_3x4;
        return true;
    }
    
    // 6x1 row
    if (W == H * 6 && W % 6 == 0) {
        S = H;
        L = CubeLayout::ROW_6x1;
        return true;
    }
    
    // 1x6 column
    if (H == W * 6 && H % 6 == 0) {
        S = W;
        L = CubeLayout::COLUMN_1x6;
        return true;
    }
    
    // Single 512x512 (special case)
    if (W == 512 && H == 512) {
        S = 512;
        L = CubeLayout::SINGLE_512x512;
        return true;
    }
    
    return false;
}

CubeFaces atlasSliceToCube(const std::vector<unsigned char>& rgba, int W, int H, int S, CubeLayout L) {
    CubeFaces faces;
    faces.size = S;
    
    if (L == CubeLayout::SINGLE_512x512) {
        // Crear 6 caras idénticas desde la imagen única
        for (int i = 0; i < 6; ++i) {
            faces.face[i].w = S;
            faces.face[i].h = S;
            faces.face[i].pixels = rgba; // Copia la misma imagen para todas las caras
        }
    } 
    else if (L == CubeLayout::HORIZONTAL_CROSS_4x3) {
        // Layout 4x3: extraer cada cara del atlas
        // Posiciones en el atlas 4x3:
        //     [T]
        // [L] [F] [R] [B]
        //     [D]
        
        struct FacePos { int x, y; };
        FacePos positions[6] = {
            {2, 1}, // +X (Right)
            {0, 1}, // -X (Left) 
            {1, 0}, // +Y (Top)
            {1, 2}, // -Y (Bottom)
            {1, 1}, // +Z (Front)
            {3, 1}  // -Z (Back)
        };
        
        for (int i = 0; i < 6; ++i) {
            faces.face[i].w = S;
            faces.face[i].h = S;
            faces.face[i].pixels.resize(S * S * 4);
            
            int srcX = positions[i].x * S;
            int srcY = positions[i].y * S;
            
            // Copiar píxeles de la cara
            for (int y = 0; y < S; ++y) {
                for (int x = 0; x < S; ++x) {
                    int srcIdx = ((srcY + y) * W + (srcX + x)) * 4;
                    int dstIdx = (y * S + x) * 4;
                    
                    for (int c = 0; c < 4; ++c) {
                        faces.face[i].pixels[dstIdx + c] = rgba[srcIdx + c];
                    }
                }
            }
        }
    } 
    else {
        throw std::runtime_error("Layout not implemented yet");
    }
    
    return faces;
}

void rotate90CW(ImageRGBA& img) {
    // Implementación simple de rotación
    std::vector<unsigned char> rotated(img.pixels.size());
    int w = img.w, h = img.h;
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * 4;
            int dstIdx = (x * h + (h - 1 - y)) * 4;
            
            for (int c = 0; c < 4; ++c) {
                rotated[dstIdx + c] = img.pixels[srcIdx + c];
            }
        }
    }
    
    img.pixels = std::move(rotated);
    std::swap(img.w, img.h);
}

void rotate90CCW(ImageRGBA& img) {
    // Implementación simple de rotación
    std::vector<unsigned char> rotated(img.pixels.size());
    int w = img.w, h = img.h;
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int srcIdx = (y * w + x) * 4;
            int dstIdx = ((w - 1 - x) * h + y) * 4;
            
            for (int c = 0; c < 4; ++c) {
                rotated[dstIdx + c] = img.pixels[srcIdx + c];
            }
        }
    }
    
    img.pixels = std::move(rotated);
    std::swap(img.w, img.h);
}

void flipVertical(ImageRGBA& img) {
    int w = img.w, h = img.h;
    
    for (int y = 0; y < h / 2; ++y) {
        for (int x = 0; x < w; ++x) {
            int topIdx = (y * w + x) * 4;
            int bottomIdx = ((h - 1 - y) * w + x) * 4;
            
            for (int c = 0; c < 4; ++c) {
                std::swap(img.pixels[topIdx + c], img.pixels[bottomIdx + c]);
            }
        }
    }
}

} // namespace util
