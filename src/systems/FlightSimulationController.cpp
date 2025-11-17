#include "systems/FlightSimulationController.h"

#include <algorithm>

extern "C" {
#include <GLFW/glfw3.h>
}

#include <cmath>

namespace systems {

FlightSimulationController::FlightSimulationController() = default;

void FlightSimulationController::initialize() {
    simulation_.initialize();
    flightData_ = simulation_.getFlightData();
    planePos_ = simulation_.getWorldPosition();
    planeOrientation_ = simulation_.getWorldOrientation();
    planeSpeed_ = simulation_.getTrueAirspeed();
    filteredJoystick_ = joystick_;
    filteredThrottle_ = throttleInput_;
}

void FlightSimulationController::resetForMission() {
    simulation_.initialize();
    joystick_ = VirtualJoystick{};
    filteredJoystick_ = joystick_;
    throttleInput_ = 0.32f;
    filteredThrottle_ = throttleInput_;
    planePos_ = simulation_.getWorldPosition();
    planeOrientation_ = simulation_.getWorldOrientation();
    planeSpeed_ = simulation_.getTrueAirspeed();
    flightData_ = simulation_.getFlightData();
}

void FlightSimulationController::handleControls(GLFWwindow* window, float dt) {
    if (!window || dt <= 0.0f) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        joystick_.aileron = moveControl(joystick_.aileron, -1.0f, controlFactor_.x, dt);
    } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        joystick_.aileron = moveControl(joystick_.aileron, +1.0f, controlFactor_.x, dt);
    } else {
        joystick_.aileron = centerControl(joystick_.aileron, controlFactor_.x, dt);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        joystick_.elevator = moveControl(joystick_.elevator, +1.0f, controlFactor_.z, dt);
    } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        joystick_.elevator = moveControl(joystick_.elevator, -1.0f, controlFactor_.z, dt);
    } else {
        joystick_.elevator = centerControl(joystick_.elevator, controlFactor_.z * 3.0f, dt);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        joystick_.rudder = moveControl(joystick_.rudder, +1.0f, controlFactor_.y, dt);
    } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        joystick_.rudder = moveControl(joystick_.rudder, -1.0f, controlFactor_.y, dt);
    } else {
        joystick_.rudder = centerControl(joystick_.rudder, controlFactor_.y, dt);
    }

    const float kThrottleRate = 2.0f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        throttleInput_ = glm::clamp(throttleInput_ + kThrottleRate * dt, 0.0f, 1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        throttleInput_ = glm::clamp(throttleInput_ - kThrottleRate * dt, 0.0f, 1.0f);
    }
}

void FlightSimulationController::step(float dt) {
    if (dt <= 0.0f) {
        return;
    }

    filteredJoystick_.aileron = smoothControlTowards(filteredJoystick_.aileron, joystick_.aileron, responseRates_.roll, dt);
    filteredJoystick_.elevator = smoothControlTowards(filteredJoystick_.elevator, joystick_.elevator, responseRates_.pitch, dt);
    filteredJoystick_.rudder = smoothControlTowards(filteredJoystick_.rudder, joystick_.rudder, responseRates_.yaw, dt);
    filteredThrottle_ = smoothControlTowards(filteredThrottle_, throttleInput_, responseRates_.throttle, dt);

    simulation_.setNormalizedInputs(filteredJoystick_.elevator,
                                    filteredJoystick_.aileron,
                                    filteredJoystick_.rudder,
                                    filteredThrottle_);

    simulation_.update(dt);

    planePos_ = simulation_.getWorldPosition();
    planeOrientation_ = simulation_.getWorldOrientation();
    planeSpeed_ = simulation_.getTrueAirspeed();
    flightData_ = simulation_.getFlightData();
}

void FlightSimulationController::setThrottle(float value) {
    throttleInput_ = glm::clamp(value, 0.0f, 1.0f);
}

float FlightSimulationController::centerControl(float value, float factor, float dt) {
    if (value >= 0.0f) {
        return glm::clamp(value - factor * dt, 0.0f, 1.0f);
    }
    return glm::clamp(value + factor * dt, -1.0f, 0.0f);
}

float FlightSimulationController::moveControl(float value, float direction, float factor, float dt) {
    return glm::clamp(value + direction * factor * dt, -1.0f, 1.0f);
}

float FlightSimulationController::smoothControlTowards(float current, float target, float responseRate, float dt) {
    if (responseRate <= 0.0f || dt <= 0.0f) {
        return target;
    }
    float alpha = 1.0f - std::exp(-responseRate * dt);
    alpha = glm::clamp(alpha, 0.0f, 1.0f);
    return glm::mix(current, target, alpha);
}

} // namespace systems
