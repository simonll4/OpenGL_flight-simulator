/**
 * @file Model.h
 * @brief 3D model loader using Assimp.
 */

#pragma once

#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

namespace gfx
{
    class Shader;
}

/**
 * @class Model
 * @brief Loads and renders 3D models using Assimp.
 *
 * Handles loading of model files (OBJ, FBX, GLTF, etc.), processing of nodes and meshes,
 * and loading of material textures.
 */
class Model
{
public:
    /**
     * @brief Constructs a Model from a file path.
     * @param path Path to the model file.
     * @param gamma Whether to apply gamma correction (not fully implemented).
     */
    Model(std::string const &path, bool gamma = false);

    /**
     * @brief Draws the model using the provided shader.
     * @param shader Shader program to use for rendering.
     */
    void Draw(gfx::Shader &shader);

private:
    // Model data
    std::vector<Texture> textures_loaded; ///< Stores all the textures loaded so far to avoid duplicates.
    std::vector<Mesh> meshes;             ///< List of meshes that make up the model.
    std::string directory;                ///< Directory path of the model file.
    bool gammaCorrection;                 ///< Gamma correction flag.

    /**
     * @brief Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
     * @param path File path to the model.
     */
    void loadModel(std::string const &path);

    /**
     * @brief Processes a node in a recursive fashion.
     * @param node Assimp node to process.
     * @param scene Assimp scene object.
     */
    void processNode(aiNode *node, const aiScene *scene);

    /**
     * @brief Processes an Assimp mesh object into a Mesh object.
     * @param mesh Assimp mesh object.
     * @param scene Assimp scene object.
     * @return Mesh Created Mesh object.
     */
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    /**
     * @brief Checks all material textures of a given type and loads the textures if they haven't been loaded yet.
     * @param mat Assimp material.
     * @param type Assimp texture type.
     * @param typeName String representation of the texture type.
     * @return std::vector<Texture> List of loaded textures.
     */
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName);
};

/**
 * @brief Utility function to load a texture from a file.
 * @param path Relative path to the texture file.
 * @param directory Directory where the model is located.
 * @param gamma Gamma correction flag.
 * @return unsigned int OpenGL texture ID.
 */
unsigned int TextureFromFile(const char *path, const std::string &directory, bool gamma = false);

#endif