/**
 * @file CameraRig.h
 * @brief Camera system with multiple view modes (first-person, third-person, cinematic).
 */

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

struct GLFWwindow;

namespace systems
{

    enum class CameraMode
    {
        FirstPerson,
        ThirdPerson,
        Cinematic
    };

    /**
     * @class CameraRig
     * @brief Controla las cámaras en primera, tercera persona y cinematográfica.
     *
     * Lee input del usuario para alternar vista y aplicar zoom.
     * Calcula matrices de vista/proyección dinámicas.
     */
    class CameraRig
    {
    public:
        /**
         * @brief Coloca la cámara siguiendo el estado inicial del avión.
         */
        void initialize(const glm::vec3 &planePos, const glm::quat &planeOrientation);
        /**
         * @brief Procesa eventos de teclado (cambio de vista, zoom).
         */
        void handleInput(GLFWwindow *window, float dt);
        /**
         * @brief Recalcula la posición/orientación de la cámara para este frame.
         */
        void update(float dt, const glm::vec3 &planePos, const glm::quat &planeOrientation, float planeSpeed);

        const glm::mat4 &viewMatrix() const { return viewMatrix_; }
        /// Matriz de proyección con far plane adaptado a la altura actual.
        glm::mat4 projectionMatrix(int width, int height) const;

        const glm::vec3 &position() const { return cameraPos_; }
        const glm::vec3 &front() const { return cameraFront_; }
        const glm::vec3 &up() const { return cameraUp_; }

        float dynamicFarPlane() const { return dynamicFarPlane_; }
        bool isFirstPerson() const { return currentMode_ == CameraMode::FirstPerson; }

        /// Reinicia desplazamientos/toggles tras cargar una misión nueva.
        void reset(const glm::vec3 &planePos, const glm::quat &planeOrientation);

    private:
        /// Vista de cockpit (rígida).
        void updateFirstPerson(const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up);
        /// Vista trasera (rígida).
        void updateThirdPerson(const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up);
        /// Vista cinematográfica (fija en puntos del mundo, siguiendo al avión).
        void updateCinematic(const glm::vec3 &planePos);

        glm::vec3 cameraPos_ = glm::vec3(0.0f);
        glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);

        float cameraDistance_ = 20.0f;

        CameraMode currentMode_ = CameraMode::ThirdPerson;

        glm::mat4 viewMatrix_ = glm::mat4(1.0f);
        float dynamicFarPlane_ = 5000.0f;

        bool toggleViewPressed_ = false;

        // Cinematic mode state
        std::vector<glm::vec3> cinematicPoints_;
        int currentCinematicIndex_ = 0;
    };

} // namespace systems
