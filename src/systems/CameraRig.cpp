#include "systems/CameraRig.h"

extern "C"
{
#include <GLFW/glfw3.h>
}

#include <algorithm>

namespace systems
{

    void CameraRig::initialize(const glm::vec3 &planePos, const glm::quat &planeOrientation)
    {
        reset(planePos, planeOrientation);
    }

    void CameraRig::reset(const glm::vec3 &planePos, const glm::quat &planeOrientation)
    {
        cameraDistance_ = 20.0f;
        currentMode_ = CameraMode::ThirdPerson;
        cameraFront_ = glm::vec3(0.0f, 0.0f, -1.0f);
        cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);

        // Regenerate cinematic points relative to the starting position
        cinematicPoints_.clear();

        // Runway/Takeoff views - Closer now
        cinematicPoints_.push_back(planePos + glm::vec3(150.0f, 30.0f, 150.0f));
        cinematicPoints_.push_back(planePos + glm::vec3(-150.0f, 20.0f, -150.0f));

        // Mid-distance views - Also closer
        cinematicPoints_.push_back(planePos + glm::vec3(300.0f, 50.0f, 300.0f));
        cinematicPoints_.push_back(planePos + glm::vec3(-300.0f, 80.0f, 100.0f));
        cinematicPoints_.push_back(planePos + glm::vec3(100.0f, 40.0f, -300.0f));

        currentCinematicIndex_ = 0;

        update(0.0f, planePos, planeOrientation, 0.0f);
    }

    void CameraRig::handleInput(GLFWwindow *window, float dt)
    {
        if (!window)
        {
            return;
        }

        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        {
            if (!toggleViewPressed_)
            {
                // Cycle modes: Third -> First -> Cinematic -> Third
                if (currentMode_ == CameraMode::ThirdPerson)
                {
                    currentMode_ = CameraMode::FirstPerson;
                }
                else if (currentMode_ == CameraMode::FirstPerson)
                {
                    currentMode_ = CameraMode::Cinematic;
                }
                else
                {
                    currentMode_ = CameraMode::ThirdPerson;
                }
                toggleViewPressed_ = true;
            }
        }
        else
        {
            toggleViewPressed_ = false;
        }

        const float kZoomSpeed = 10.0f;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            cameraDistance_ = glm::clamp(cameraDistance_ - kZoomSpeed * dt, 10.0f, 50.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            cameraDistance_ = glm::clamp(cameraDistance_ + kZoomSpeed * dt, 10.0f, 50.0f);
        }
    }

    void CameraRig::update(float dt, const glm::vec3 &planePos, const glm::quat &planeOrientation, float planeSpeed)
    {
        glm::vec3 forward = planeOrientation * glm::vec3(0, 0, -1);
        glm::vec3 up = planeOrientation * glm::vec3(0, 1, 0);

        switch (currentMode_)
        {
        case CameraMode::FirstPerson:
            updateFirstPerson(planePos, forward, up);
            break;
        case CameraMode::ThirdPerson:
            updateThirdPerson(planePos, forward, up);
            break;
        case CameraMode::Cinematic:
            updateCinematic(planePos);
            break;
        }

        viewMatrix_ = glm::lookAt(cameraPos_, cameraPos_ + cameraFront_, cameraUp_);

        float cameraHeight = cameraPos_.y;
        dynamicFarPlane_ = std::max(5000.0f, cameraHeight * 20.0f);
        dynamicFarPlane_ = std::min(dynamicFarPlane_, 500000.0f);
    }

    glm::mat4 CameraRig::projectionMatrix(int width, int height) const
    {
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, dynamicFarPlane_);
    }

    void CameraRig::updateFirstPerson(const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up)
    {
        // Rigid cockpit view
        cameraPos_ = planePos + forward * 6.0f + up * 1.8f;
        cameraFront_ = forward;
        cameraUp_ = up;
    }

    void CameraRig::updateThirdPerson(const glm::vec3 &planePos, const glm::vec3 &forward, const glm::vec3 &up)
    {
        // Rigid third person view
        glm::vec3 targetPos = planePos - forward * cameraDistance_ + up * (cameraDistance_ * 0.4f);
        glm::vec3 lookTarget = planePos + forward * 5.0f;

        cameraPos_ = targetPos;
        cameraFront_ = glm::normalize(lookTarget - targetPos);
        cameraUp_ = up;
    }

    void CameraRig::updateCinematic(const glm::vec3 &planePos)
    {
        // Dynamic Cinematic Logic:
        // 1. If we have no points, or the plane is too far from ALL points, generate a new one near the plane.
        // 2. Switch to the best existing point if available.

        float kMaxDistance = 300.0f;   // If plane is further than this, we need a new camera.
        float kSpawnDistance = 150.0f; // Distance to spawn new camera from plane.

        // Check distance to current camera
        float currentDist = 100000.0f;
        if (!cinematicPoints_.empty())
        {
            currentDist = glm::distance(planePos, cinematicPoints_[currentCinematicIndex_]);
        }

        // If we are too far, or have no points, try to find a better existing point
        int bestIndex = -1;
        float bestDist = 100000.0f;

        for (size_t i = 0; i < cinematicPoints_.size(); ++i)
        {
            float d = glm::distance(planePos, cinematicPoints_[i]);
            if (d < bestDist)
            {
                bestDist = d;
                bestIndex = static_cast<int>(i);
            }
        }

        // If even the best point is too far, generate a new one!
        if (bestDist > kMaxDistance || cinematicPoints_.empty())
        {
            // Refined approach:
            // Spawn a point 300m away in a somewhat random direction
            float theta = static_cast<float>(rand() % 360);
            float phi = static_cast<float>(rand() % 45 + 10); // 10 to 55 degrees up

            float x = kSpawnDistance * std::cos(glm::radians(phi)) * std::cos(glm::radians(theta));
            float y = kSpawnDistance * std::sin(glm::radians(phi));
            float z = kSpawnDistance * std::cos(glm::radians(phi)) * std::sin(glm::radians(theta));

            glm::vec3 spawnPos = planePos + glm::vec3(x, y, z);

            cinematicPoints_.push_back(spawnPos);
            bestIndex = cinematicPoints_.size() - 1;
            bestDist = glm::distance(planePos, spawnPos);
        }

        // Hysteresis for switching
        if (bestIndex != -1 && bestIndex != currentCinematicIndex_)
        {
            float distToBest = glm::distance(planePos, cinematicPoints_[bestIndex]);

            // Switch if the new point is significantly closer (e.g. 70% of current distance)
            // OR if the current distance is getting too large (> 600m)
            if (distToBest < currentDist * 0.7f || currentDist > kMaxDistance * 0.8f)
            {
                currentCinematicIndex_ = bestIndex;
            }
        }

        // Safety check
        if (currentCinematicIndex_ >= 0 && currentCinematicIndex_ < static_cast<int>(cinematicPoints_.size()))
        {
            cameraPos_ = cinematicPoints_[currentCinematicIndex_];
            cameraFront_ = glm::normalize(planePos - cameraPos_);
            cameraUp_ = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

} // namespace systems
