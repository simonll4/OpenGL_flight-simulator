/**
 * @file Shader.h
 * @brief OpenGL shader program wrapper.
 */

#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

extern "C"
{
#include <glad/glad.h>
}

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gfx
{

    /**
     * @brief Manages the creation and usage of an OpenGL program composed of
     *        vertex and fragment shaders.
     *
     * The wrapper handles loading from files, compilation, error checking,
     * linking, and subsequent destruction of the program. It is designed as a movable
     * resource to allow ownership transfer without duplicating the GL handle.
     */
    class Shader
    {
    public:
        Shader() = default;
        /// Convenience: constructs and compiles immediately from the given paths.
        Shader(const char *vsPath, const char *fsPath) { load(vsPath, fsPath); }
        ~Shader()
        {
            if (prog_)
                glDeleteProgram(prog_);
        }

        /// Non-copyable: the OpenGL program should not be destroyed multiple times.
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        /// Allows moving the program (transferring the handle) between instances.
        Shader(Shader &&other) noexcept : prog_(other.prog_) { other.prog_ = 0; }
        Shader &operator=(Shader &&other) noexcept
        {
            if (this != &other)
            {
                if (prog_)
                    glDeleteProgram(prog_);
                prog_ = other.prog_;
                other.prog_ = 0;
            }
            return *this;
        }

        /**
         * @brief Loads, compiles, and links shaders located at the given paths.
         * @throws std::runtime_error if any step fails (I/O, compilation, or link).
         */
        void load(const char *vsPath, const char *fsPath);

        /// Makes this program active in the pipeline.
        void use() const { glUseProgram(prog_); }

        /// Direct exposure of the GL identifier (read-only).
        GLuint id() const { return prog_; }

        // Setters for typical uniforms used in the engine.
        void setMat4(const char *name, const glm::mat4 &m) const;
        void setInt(const char *name, int v) const;
        void setBool(const char *name, bool v) const;
        void setFloat(const char *name, float v) const;
        void setVec3(const char *name, const glm::vec3 &v) const;

    private:
        GLuint prog_ = 0; ///< OpenGL program ID (0 indicates uninitialized).

        std::string readFile(const char *path);
        GLuint compileShader(const std::string &source, GLenum type);
        void checkCompileErrors(GLuint shader, const std::string &type);
    };

} // namespace gfx
