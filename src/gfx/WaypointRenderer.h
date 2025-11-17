#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "core/Shader.h"

extern "C"
{
#include <glad/glad.h>
}

namespace gfx
{
    /**
     * @class WaypointRenderer
     * @brief Renderiza waypoints como marcadores visuales 3D en el mundo.
     *
     * Genera un cilindro vertical estilizado que se posiciona en cada waypoint
     * y lo colorea en función de su estado (activo/inactivo). El renderer mantiene
     * la geometría en GPU y solo actualiza matrices/uniformes por draw call.
     */
    class WaypointRenderer
    {
    public:
        WaypointRenderer() = default;
        ~WaypointRenderer(); ///< Libera los buffers GL asociados al cilindro.

        /**
         * @brief Compila el shader y crea la geometría cilindro.
         * @throws std::runtime_error si el shader no puede cargarse.
         */
        void init();

        /**
         * @brief Dibuja un waypoint usando matrices de vista/proyección provistas.
         * @param view Matriz de vista (camara).
         * @param proj Matriz de proyección (perspectiva).
         * @param position Posición mundial del waypoint.
         * @param color Color RGBA emitido; alpha controla brillo.
         * @param isActive Resalta el marcador cuando es el waypoint actual.
         */
        void drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                          const glm::vec3 &position, const glm::vec4 &color,
                          bool isActive = false);

    private:
        GLuint vao_ = 0, vbo_ = 0, ebo_ = 0; ///< Buffers para la malla del cilindro.
        Shader shader_;                      ///< Shader que aplica iluminación simple.
        int indexCount_ = 0;                 ///< Número de índices a renderizar.

        /// Genera el cilindro unitario y configura atributos de vértice (pos/normal).
        void createCylinderGeometry();
    };

} // namespace gfx
