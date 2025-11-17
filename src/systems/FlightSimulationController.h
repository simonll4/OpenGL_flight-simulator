#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "flight/data/FlightData.h"
#include "flight/dlfdm/FdmSimulation.h"

struct GLFWwindow;

namespace systems {

class FlightSimulationController {
public:
    FlightSimulationController();

    void initialize();
    void resetForMission();
    void handleControls(GLFWwindow* window, float dt);
    void step(float dt);

    const flight::FlightData& getFlightData() const { return flightData_; }
    flight::FlightData& getFlightData() { return flightData_; }
    const glm::vec3& planePosition() const { return planePos_; }
    const glm::quat& planeOrientation() const { return planeOrientation_; }
    float planeSpeed() const { return planeSpeed_; }
    float throttleInput() const { return throttleInput_; }

    void setThrottle(float value);

private:
    struct VirtualJoystick {
        float aileron = 0.0f;
        float elevator = 0.0f;
        float rudder = 0.0f;
    };

    float centerControl(float value, float factor, float dt);
    float moveControl(float value, float direction, float factor, float dt);
    float smoothControlTowards(float current, float target, float responseRate, float dt);

    flight::FdmSimulation simulation_;
    flight::FlightData flightData_;

    glm::vec3 planePos_ = glm::vec3(0.0f, 1500.0f, 0.0f);
    glm::quat planeOrientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    float planeSpeed_ = 0.0f;

    VirtualJoystick joystick_{};
    VirtualJoystick filteredJoystick_{};

    glm::vec3 controlFactor_ = glm::vec3(3.0f, 1.0f, 3.0f);

    struct ControlResponseRates {
        float roll;
        float pitch;
        float yaw;
        float throttle;
    } responseRates_{8.0f, 6.0f, 4.0f, 3.0f};

    float throttleInput_ = 0.32f;
    float filteredThrottle_ = throttleInput_;
};

} // namespace systems
