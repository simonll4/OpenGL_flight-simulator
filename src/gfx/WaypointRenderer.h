/**
 * @file WaypointRenderer.h
 * @brief 3D waypoint marker renderer.
 */

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
     * @brief Renders waypoints as 3D visual markers in the world.
     *
     * Generates a stylized vertical cylinder positioned at each waypoint
     * and colors it based on its state (active/inactive). The renderer maintains
     * the geometry on the GPU and only updates matrices/uniforms per draw call.
     */
    class WaypointRenderer
    {
    public:
        WaypointRenderer() = default;
        ~WaypointRenderer(); ///< Releases associated GL buffers.

        /**
         * @brief Compiles the shader and creates the cylinder geometry.
         * @throws std::runtime_error if the shader cannot be loaded.
         */
        void init();

        /**
         * @brief Draws a waypoint using the provided view/projection matrices.
         * @param view View matrix (camera).
         * @param proj Projection matrix (perspective).
         * @param position World position of the waypoint.
         * @param color RGBA color emitted; alpha controls brightness.
         * @param isActive Highlights the marker when it is the current waypoint.
         */
        void drawWaypoint(const glm::mat4 &view, const glm::mat4 &proj,
                          const glm::vec3 &position, const glm::vec4 &color,
                          bool isActive = false);

    private:
        GLuint vao_ = 0, vbo_ = 0, ebo_ = 0; ///< Buffers for the cylinder mesh.
        Shader shader_;                      ///< Shader applying simple lighting.
        int indexCount_ = 0;                 ///< Number of indices to render.

        /// Generates the unit cylinder and configures vertex attributes (pos/normal).
        void createCylinderGeometry();
    };

} // namespace gfx
