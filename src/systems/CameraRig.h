#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

struct GLFWwindow;

namespace systems {

class CameraRig {
public:
    void initialize(const glm::vec3& planePos, const glm::quat& planeOrientation);
    void handleInput(GLFWwindow* window, float dt);
    void update(float dt, const glm::vec3& planePos, const glm::quat& planeOrientation, float planeSpeed);

    const glm::mat4& viewMatrix() const { return viewMatrix_; }
    glm::mat4 projectionMatrix(int width, int height) const;

    const glm::vec3& position() const { return cameraPos_; }
    const glm::vec3& front() const { return cameraFront_; }
    const glm::vec3& up() const { return cameraUp_; }

    float dynamicFarPlane() const { return dynamicFarPlane_; }
    bool isFirstPerson() const { return firstPersonView_; }
    bool isSmoothCamera() const { return smoothCamera_; }

    void reset(const glm::vec3& planePos, const glm::quat& planeOrientation);

private:
    void updateFirstPerson(float dt, const glm::vec3& planePos, const glm::vec3& forward, const glm::vec3& up);
    void updateThirdPerson(float dt, const glm::vec3& planePos, const glm::vec3& forward, const glm::vec3& up, float planeSpeed);

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
