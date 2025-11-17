#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct GLFWwindow;

namespace systems
{

    /**
     * @class CameraRig
     * @brief Controla las cámaras en primera y tercera persona alrededor del avión.
     *
     * Lee input del usuario para alternar vista, aplicar zoom y activar suavizado.
     * Calcula matrices de vista/proyección dinámicas que se alimentan al renderer
     * teniendo en cuenta posición/orientación de la aeronave y velocidad actual.
     */
    class CameraRig
    {
    public:
        /**
         * @brief Coloca la cámara siguiendo el estado inicial del avión.
         */
        void initialize(const glm::vec3 &planePos, const glm::quat &planeOrientation);
        /**
         * @brief Procesa eventos de teclado (cambio de vista, suavizado, zoom).
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
        bool isFirstPerson() const { return firstPersonView_; }
        bool isSmoothCamera() const { return smoothCamera_; }

        /// Reinicia desplazamientos/toggles tras cargar una misión nueva.
        void reset(const glm::vec3 &planePos, const glm::quat &planeOrientation);

    private:
        /// Interpola hacia el cockpit respetando el modo de suavizado.
        void updateFirstPerson(float dt, const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up);
        /// Posiciona la cámara detrás del avión con offset dependiente de velocidad.
        void updateThirdPerson(float dt, const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up, float planeSpeed);

        glm::vec3 cameraPos_ = glm::vec3(0.0f);
        glm::vec3 cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);

        float cameraDistance_ = 20.0f;
        float cameraLerpSpeed_ = 5.0f;
        bool firstPersonView_ = false;
        bool smoothCamera_ = true;

        glm::mat4 viewMatrix_ = glm::mat4(1.0f);
        float dynamicFarPlane_ = 5000.0f;

        bool toggleFirstPersonPressed_ = false;
        bool toggleSmoothPressed_ = false;
    };

} // namespace systems
