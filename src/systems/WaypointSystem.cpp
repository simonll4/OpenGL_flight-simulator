#include "systems/WaypointSystem.h"

#include <iostream>
#include <memory>

extern "C" {
#include <glad/glad.h>
}

#include <glm/gtc/constants.hpp>

#include "gfx/WaypointRenderer.h"

namespace systems {

WaypointSystem::WaypointSystem() = default;
WaypointSystem::~WaypointSystem() = default;

void WaypointSystem::initialize() {
    renderer_ = std::make_unique<gfx::WaypointRenderer>();
    renderer_->init();
}

void WaypointSystem::reset() {
    waypoints_.clear();
}

void WaypointSystem::loadFromMission(const mission::MissionDefinition& mission) {
    reset();
    waypoints_.reserve(mission.waypoints.size());
    for (const auto& wp : mission.waypoints) {
        waypoints_.push_back({wp.position, wp.name, false});
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "      🎯 " << mission.name << " 🎯" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Descripción: " << mission.description << std::endl;
    std::cout << "Waypoints: " << waypoints_.size() << " cargados" << std::endl;
    std::cout << "Categoría: " << mission.category << std::endl;
    std::cout << "Dificultad: " << mission.difficulty << "/5" << std::endl;

    if (!mission.environment.timeOfDay.empty()) {
        std::cout << "Hora: " << mission.environment.timeOfDay << std::endl;
    }
    if (!mission.environment.weather.empty()) {
        std::cout << "Clima: " << mission.environment.weather << std::endl;
    }

    std::cout << "\nControles de Vuelo:" << std::endl;
    std::cout << "  W/S       - Pitch (arriba/abajo)" << std::endl;
    std::cout << "  A/D       - Roll (rodar izq/der)" << std::endl;
    std::cout << "  Q/E       - Yaw (girar izq/der)" << std::endl;
    std::cout << "  UP/DOWN   - Aumentar/reducir velocidad" << std::endl;
    std::cout << "\nControles de Cámara:" << std::endl;
    std::cout << "  V         - Cambiar vista POV/3ra persona" << std::endl;
    std::cout << "  C         - Activar/Desactivar cámara suave" << std::endl;
    std::cout << "  Z/X       - Alejar/Acercar cámara" << std::endl;
    std::cout << "\nControles de Misión:" << std::endl;
    std::cout << "  N         - Activar/Desactivar waypoints" << std::endl;
    std::cout << "  M         - Saltar waypoint actual" << std::endl;
    std::cout << "  R         - Reiniciar misión" << std::endl;
    std::cout << "  TAB       - Volver al menú de misiones" << std::endl;
    std::cout << "  ESC       - Salir del simulador" << std::endl;
    std::cout << "========================================\n" << std::endl;
}

void WaypointSystem::update(const glm::vec3& planePos,
                             flight::FlightData& flightData,
                             mission::MissionRuntime& runtime) {
    if (!runtime.areWaypointsEnabled() || waypoints_.empty()) {
        flightData.hasActiveWaypoint = false;
        return;
    }

    int nextWaypointIndex = -1;
    for (size_t i = 0; i < waypoints_.size(); ++i) {
        if (!waypoints_[i].captured) {
            nextWaypointIndex = static_cast<int>(i);
            break;
        }
    }

    if (nextWaypointIndex == -1) {
        if (!runtime.isCompleted()) {
            runtime.markCompletion();
        }
        flightData.hasActiveWaypoint = false;
        return;
    }

    const auto& currentWaypoint = waypoints_[nextWaypointIndex];
    flightData.targetWaypoint = currentWaypoint.position;
    flightData.hasActiveWaypoint = true;

    glm::vec3 toWaypoint = currentWaypoint.position - planePos;
    flightData.waypointDistance = glm::length(toWaypoint);

    glm::vec2 toWaypointXZ = glm::vec2(toWaypoint.x, toWaypoint.z);
    if (glm::length(toWaypointXZ) > 0.01f) {
    float bearing = atan2(toWaypointXZ.x, -toWaypointXZ.y) * (180.0f / glm::pi<float>());
        if (bearing < 0.0f) {
            bearing += 360.0f;
        }
        flightData.waypointBearing = bearing;
    }

    const float kCaptureRadius = 100.0f;
    if (flightData.waypointDistance < kCaptureRadius) {
        waypoints_[nextWaypointIndex].captured = true;
        runtime.markWaypointCaptured(nextWaypointIndex);

        int remaining = 0;
        for (const auto& wp : waypoints_) {
            if (!wp.captured) {
                remaining++;
            }
        }

        std::cout << "✓ Waypoint " << currentWaypoint.name << " alcanzado! ";
        if (remaining > 0) {
            std::cout << "Waypoints restantes: " << remaining << std::endl;
        } else {
            std::cout << "¡Último waypoint!" << std::endl;
        }
    }
}

void WaypointSystem::render(const glm::mat4& view,
                             const glm::mat4& projection,
                             const mission::MissionRuntime& runtime) const {
    if (!renderer_ || waypoints_.empty() || !runtime.areWaypointsEnabled()) {
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < waypoints_.size(); ++i) {
        if (waypoints_[i].captured) {
            continue;
        }

        int activeIdx = runtime.getActiveWaypointIndex();
        bool isActive = static_cast<int>(i) == activeIdx;
        glm::vec4 color = isActive
                               ? glm::vec4(0.0f, 1.0f, 0.4f, 0.8f)
                               : glm::vec4(0.2f, 0.5f, 1.0f, 0.6f);

        renderer_->drawWaypoint(view, projection, waypoints_[i].position, color, isActive);
    }

    glDisable(GL_BLEND);
}

void WaypointSystem::skipActiveWaypoint(mission::MissionRuntime& runtime) {
    int activeIdx = runtime.getActiveWaypointIndex();
    if (activeIdx < 0 || activeIdx >= static_cast<int>(waypoints_.size())) {
        return;
    }

    if (!waypoints_[activeIdx].captured) {
        waypoints_[activeIdx].captured = true;
        runtime.markWaypointCaptured(activeIdx);
        std::cout << "Waypoint " << waypoints_[activeIdx].name << " saltado manualmente" << std::endl;
    }
}

} // namespace systems
