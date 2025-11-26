/**
 * @file ImageAtlas.h
 * @brief Utilities for loading and processing cubemap atlas images.
 *
 * This module provides functionality to load skybox/cubemap textures from
 * various atlas layouts and convert them to individual cube faces. Supports
 * multiple common atlas configurations used in graphics applications.
 */

#pragma once
#include <vector>
#include <string>

namespace util
{

    /**
     * @enum CubeLayout
     * @brief Supported atlas layout configurations for cubemap images.
     *
     * Defines how the 6 cube faces are arranged in a single image file.
     */
    enum class CubeLayout
    {
        HORIZONTAL_CROSS_4x3, ///< Horizontal cross pattern (4 columns, 3 rows).
        VERTICAL_CROSS_3x4,   ///< Vertical cross pattern (3 columns, 4 rows).
        ROW_6x1,              ///< Horizontal row of 6 faces (6 columns, 1 row).
        COLUMN_1x6,           ///< Vertical column of 6 faces (1 column, 6 rows).
        SINGLE_512x512        ///< Single face image (for testing/debugging).
    };

    /**
     * @struct ImageRGBA
     * @brief Container for RGBA image data.
     */
    struct ImageRGBA
    {
        std::vector<unsigned char> pixels; ///< Raw RGBA pixel data (4 bytes per pixel).
        int w;                             ///< Image width in pixels.
        int h;                             ///< Image height in pixels.
    };

    /**
     * @struct CubeFaces
     * @brief Container for the 6 faces of a cubemap.
     *
     * Face order follows OpenGL convention:
     * [0] = +X (Right), [1] = -X (Left),
     * [2] = +Y (Top),   [3] = -Y (Bottom),
     * [4] = +Z (Front), [5] = -Z (Back)
     */
    struct CubeFaces
    {
        ImageRGBA face[6]; ///< The 6 cube faces in OpenGL order.
        int size;          ///< Size of each face (assumed square: size x size).
    };

    /**
     * @brief Loads an RGBA image from a file.
     *
     * @param path File path to the image (supports PNG, JPG, BMP, TGA via stb_image).
     * @param W Output: image width in pixels.
     * @param H Output: image height in pixels.
     * @param rgba Output: raw RGBA pixel data (4 bytes per pixel).
     * @param flipY If true, flips the image vertically (useful for OpenGL texture coordinates).
     * @return true if the image was loaded successfully, false otherwise.
     */
    bool atlasLoadRGBA(const std::string &path, int &W, int &H, std::vector<unsigned char> &rgba, bool flipY = false);

    /**
     * @brief Detects the atlas layout from image dimensions.
     *
     * Analyzes the width and height to determine which CubeLayout pattern is used
     * and calculates the size of each individual cube face.
     *
     * @param W Image width in pixels.
     * @param H Image height in pixels.
     * @param S Output: detected face size (each face is S x S pixels).
     * @param L Output: detected CubeLayout type.
     * @return true if a valid layout was detected, false if dimensions don't match any pattern.
     */
    bool atlasDetect(int W, int H, int &S, CubeLayout &L);

    /**
     * @brief Converts an atlas image to individual cubemap faces.
     *
     * Extracts the 6 cube faces from the atlas according to the specified layout,
     * applying necessary rotations and transformations to match OpenGL conventions.
     *
     * @param rgba Raw RGBA pixel data of the atlas image.
     * @param W Atlas image width.
     * @param H Atlas image height.
     * @param S Face size (each face is S x S pixels).
     * @param L Atlas layout type.
     * @return CubeFaces structure containing the 6 extracted and oriented faces.
     */
    CubeFaces atlasSliceToCube(const std::vector<unsigned char> &rgba, int W, int H, int S, CubeLayout L);

    /**
     * @brief Rotates an image 90 degrees clockwise.
     * @param img Image to rotate (modified in place).
     */
    void rotate90CW(ImageRGBA &img);

    /**
     * @brief Rotates an image 90 degrees counter-clockwise.
     * @param img Image to rotate (modified in place).
     */
    void rotate90CCW(ImageRGBA &img);

    /**
     * @brief Flips an image vertically (top to bottom).
     * @param img Image to flip (modified in place).
     */
    void flipVertical(ImageRGBA &img);

} // namespace util
