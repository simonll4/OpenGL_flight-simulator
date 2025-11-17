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
     * @brief Administra la creación y uso de un programa OpenGL compuesto por
     *        shaders de vértices y fragmentos.
     *
     * El wrapper maneja la carga desde archivos, compilación, chequeo de errores,
     * link y la posterior destrucción del programa. Se diseña como recurso movible
     * para permitir transferencia de propiedad sin duplicar el handle GL.
     */
    class Shader
    {
    public:
        Shader() = default;
        /// Conveniencia: construye y compila inmediatamente desde las rutas dadas.
        Shader(const char *vsPath, const char *fsPath) { load(vsPath, fsPath); }
        ~Shader()
        {
            if (prog_)
                glDeleteProgram(prog_);
        }

        /// No copiable: el programa OpenGL no debe destruirse múltiples veces.
        Shader(const Shader &) = delete;
        Shader &operator=(const Shader &) = delete;

        /// Permite mover el programa (transferir el handle) entre instancias.
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
         * @brief Carga, compila y linkea los shaders ubicados en las rutas dadas.
         * @throws std::runtime_error si algún paso falla (I/O, compilación o link).
         */
        void load(const char *vsPath, const char *fsPath);

        /// Hace este programa el activo en el pipeline.
        void use() const { glUseProgram(prog_); }

        /// Exposición directa del identificador GL (lectura solamente).
        GLuint id() const { return prog_; }

        // Setters para uniformes típicos utilizados en el motor.
        void setMat4(const char *name, const glm::mat4 &m) const;
        void setInt(const char *name, int v) const;
        void setBool(const char *name, bool v) const;
        void setFloat(const char *name, float v) const;
        void setVec3(const char *name, const glm::vec3 &v) const;

    private:
        GLuint prog_ = 0; ///< ID del programa en OpenGL (0 indica no inicializado).

        std::string readFile(const char *path);
        GLuint compileShader(const std::string &source, GLenum type);
        void checkCompileErrors(GLuint shader, const std::string &type);
    };

} // namespace gfx
