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
     * @brief Renderiza waypoints como marcadores visuales 3D en el mundo
     *
     * Dibuja cilindros verticales brillantes en las posiciones de los waypoints
     * para ayudar al piloto a identificar visualmente los puntos de navegación.
     */
    class WaypointRenderer
    {
    public:
        WaypointRenderer() = default;
        ~WaypointRenderer();

        void init();
        void drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                          const glm::vec3 &position, const glm::vec4 &color,
                          bool isActive = false);

    private:
        GLuint vao_ = 0, vbo_ = 0, ebo_ = 0;
        Shader shader_;
        int indexCount_ = 0;

        void createCylinderGeometry();
    };

} // namespace gfx
