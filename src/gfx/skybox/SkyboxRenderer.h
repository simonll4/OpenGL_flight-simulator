/**
 * @file SkyboxRenderer.h
 * @brief Skybox cubemap renderer.
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../core/Shader.h"
#include "TextureCube.h"

namespace gfx
{

    /**
     * @brief Encapsula la renderización del cielo tipo cubemap.
     *
     * La clase administra el VAO/VBO con la geometría del cubo unitario,
     * el shader especializado y la textura cubemap asociada. Se diseña como
     * recurso de larga vida (se crea una vez y se reutiliza en cada frame).
     */
    class SkyboxRenderer
    {
    public:
        SkyboxRenderer() = default;
        ~SkyboxRenderer();

        /// Instancia no copiable: compartir el VAO/VBO entre copias sería inseguro.
        SkyboxRenderer(const SkyboxRenderer &) = delete;
        SkyboxRenderer &operator=(const SkyboxRenderer &) = delete;

        /// Permite transferir la propiedad de los recursos OpenGL mediante movimiento.
        SkyboxRenderer(SkyboxRenderer &&other) noexcept
            : vao_(other.vao_), vbo_(other.vbo_), shader_(std::move(other.shader_)), cube_(other.cube_)
        {
            other.vao_ = other.vbo_ = 0;
            other.cube_ = nullptr;
        }

        /**
         * @brief Inicializa la geometría del cubo y compila el shader.
         * @throws std::exception si la compilación de shaders falla.
         */
        void init();

        /// Registra la textura cubemap a utilizar en los draw posteriores.
        void setCubemap(TextureCube *tex) { cube_ = tex; }

        /**
         * @brief Dibuja el skybox con las matrices de vista y proyección provistas.
         *
         * La vista se normaliza eliminando la traslación para evitar movimientos del
         * skybox al mover la cámara, manteniendo únicamente la rotación.
         */
        void draw(const glm::mat4 &view, const glm::mat4 &proj);

    private:
        GLuint vao_ = 0, vbo_ = 0;    ///< Identificadores de la geometría del cubo en GPU.
        Shader shader_;               ///< Shader responsable del muestreo del cubemap.
        TextureCube *cube_ = nullptr; ///< Textura cubemap actualmente asociada (no propiedad).

        /// Crea y configura el VAO/VBO del cubo unitario (36 vértices).
        void createCubeGeometry();
    };

} // namespace gfx
