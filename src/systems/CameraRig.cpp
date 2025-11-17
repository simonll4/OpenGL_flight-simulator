#include "systems/CameraRig.h"

extern "C" {
#include <GLFW/glfw3.h>
}

#include <algorithm>

namespace systems {

void CameraRig::initialize(const glm::vec3& planePos, const glm::quat& planeOrientation) {
    reset(planePos, planeOrientation);
}

void CameraRig::reset(const glm::vec3& planePos, const glm::quat& planeOrientation) {
    cameraDistance_ = 20.0f;
    firstPersonView_ = false;
    smoothCamera_ = true;
    cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
    update(0.0f, planePos, planeOrientation, 0.0f);
}

void CameraRig::handleInput(GLFWwindow* window, float dt) {
    if (!window) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        if (!toggleFirstPersonPressed_) {
            firstPersonView_ = !firstPersonView_;
            toggleFirstPersonPressed_ = true;
        }
    } else {
        toggleFirstPersonPressed_ = false;
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!toggleSmoothPressed_) {
            smoothCamera_ = !smoothCamera_;
            toggleSmoothPressed_ = true;
        }
    } else {
        toggleSmoothPressed_ = false;
    }

    const float kZoomSpeed = 10.0f;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        cameraDistance_ = glm::clamp(cameraDistance_ - kZoomSpeed * dt, 10.0f, 50.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        cameraDistance_ = glm::clamp(cameraDistance_ + kZoomSpeed * dt, 10.0f, 50.0f);
    }
}

void CameraRig::update(float dt, const glm::vec3& planePos, const glm::quat& planeOrientation, float planeSpeed) {
    glm::vec3 forward = planeOrientation * glm::vec3(0, 0, -1);
    glm::vec3 up = planeOrientation * glm::vec3(0, 1, 0);

    if (firstPersonView_) {
        updateFirstPerson(dt, planePos, forward, up);
    } else {
        updateThirdPerson(dt, planePos, forward, up, planeSpeed);
    }

    viewMatrix_ = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);

    float cameraHeight = cameraPos_.y;
    dynamicFarPlane_ = std::max(5000.0f, cameraHeight * 20.0f);
    dynamicFarPlane_ = std::min(dynamicFarPlane_, 500000.0f);
}

glm::mat4 CameraRig::projectionMatrix(int width, int height) const {
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    return glm::perspective(glm::radians(45.0f), aspect, 0.1f, dynamicFarPlane_);
}

void CameraRig::updateFirstPerson(float dt, const glm::vec3& planePos, const glm::vec3& forward, const glm::vec3& up) {
    glm::vec3 targetPos = planePos + forward * 6.0f + up * 1.8f;
    if (smoothCamera_) {
        float lerpFactor = glm::clamp(dt * cameraLerpSpeed_ * 2.0f, 0.0f, 1.0f);
        cameraPos_ = glm::mix(cameraPos_, targetPos, lerpFactor);
    } else {
        cameraPos_ = targetPos;
    }
    cameraFront_ = forward;
    cameraUp_ = up;
}

void CameraRig::updateThirdPerson(float dt, const glm::vec3& planePos, const glm::vec3& forward, const glm::vec3& up, float planeSpeed) {
    glm::vec3 targetPos = planePos - forward * cameraDistance_ + up * (cameraDistance_ * 0.4f);
    glm::vec3 lookTarget = planePos + forward * 5.0f;
    glm::vec3 targetFront = glm::normalize(lookTarget - targetPos);

    if (smoothCamera_) {
        float speedFactor = glm::clamp(planeSpeed / 100.0f, 0.5f, 1.5f);
        float lerpFactor = glm::clamp(dt * cameraLerpSpeed_ * speedFactor * 0.8f, 0.0f, 0.5f);
        cameraPos_ = glm::mix(cameraPos_, targetPos, lerpFactor);
        cameraFront_ = glm::normalize(glm::mix(cameraFront_, targetFront, glm::clamp(lerpFactor * 1.5f, 0.0f, 1.0f)));
    } else {
        cameraPos_ = targetPos;
        cameraFront_ = targetFront;
    }

    cameraUp_ = up;
}

} // namespace systems
