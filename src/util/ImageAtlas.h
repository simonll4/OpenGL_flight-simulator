#pragma once
#include <vector>
#include <string>

namespace util {

enum class CubeLayout {
    HORIZONTAL_CROSS_4x3,
    VERTICAL_CROSS_3x4,
    ROW_6x1,
    COLUMN_1x6,
    SINGLE_512x512
};

struct ImageRGBA {
    std::vector<unsigned char> pixels;
    int w, h;
};

struct CubeFaces {
    ImageRGBA face[6];
    int size;
};

// Cargar imagen RGBA desde archivo
bool atlasLoadRGBA(const std::string& path, int& W, int& H, std::vector<unsigned char>& rgba, bool flipY = false);

// Detectar layout del atlas
bool atlasDetect(int W, int H, int& S, CubeLayout& L);

// Convertir atlas a caras de cubemap
CubeFaces atlasSliceToCube(const std::vector<unsigned char>& rgba, int W, int H, int S, CubeLayout L);

// Utilidades de transformación
void rotate90CW(ImageRGBA& img);
void rotate90CCW(ImageRGBA& img);
void flipVertical(ImageRGBA& img);

} // namespace util
