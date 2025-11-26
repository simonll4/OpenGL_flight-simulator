/**
 * @file Mesh.h
 * @brief 3D mesh representation with vertex data and textures.
 */

#pragma once

#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace gfx
{
    class Shader;
}

struct Vertex
{
    glm::vec3 Position;  ///< Vertex position in 3D space.
    glm::vec3 Normal;    ///< Vertex normal vector.
    glm::vec2 TexCoords; ///< Texture coordinates (UV).
    glm::vec3 Tangent;   ///< Tangent vector for normal mapping.
    glm::vec3 Bitangent; ///< Bitangent vector for normal mapping.
};

struct Texture
{
    unsigned int id;  ///< OpenGL texture ID.
    std::string type; ///< Texture type (e.g., "texture_diffuse").
    std::string path; ///< Path to the texture file.
};

/**
 * @class Mesh
 * @brief Represents a renderable 3D mesh.
 *
 * Encapsulates vertex data, indices, and textures. Handles the setup of
 * OpenGL buffers (VAO, VBO, EBO) and provides a method to render itself.
 */
class Mesh
{
public:
    // Mesh Data
    std::vector<Vertex> vertices;      ///< List of vertices.
    std::vector<unsigned int> indices; ///< List of indices for drawing elements.
    std::vector<Texture> textures;     ///< List of textures associated with the mesh.
    unsigned int VAO;                  ///< Vertex Array Object ID.

    /**
     * @brief Constructs a Mesh from vertices, indices, and textures.
     * @param vertices Vector of Vertex structs.
     * @param indices Vector of indices.
     * @param textures Vector of Texture structs.
     */
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    /**
     * @brief Renders the mesh using the provided shader.
     * @param shader Shader program to use for rendering.
     */
    void Draw(gfx::Shader &shader);

private:
    // Render data
    unsigned int VBO, EBO;

    /**
     * @brief Initializes all the buffer objects/arrays.
     */
    void setupMesh();
};

#endif