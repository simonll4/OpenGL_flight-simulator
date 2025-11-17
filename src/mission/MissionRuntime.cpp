/**
 * @file MissionRuntime.cpp
 * @brief Implementación del gestor de estado de misiones
 */

#include "MissionRuntime.h"
#include "../flight/data/FlightData.h"
#include <iostream>
#include <glm/glm.hpp>

namespace mission {

MissionRuntime::MissionRuntime()
    : phase_(MissionPhase::Briefing)
    , hasMission_(false)
    , menuExitRequested_(false)
    , activeWaypointIndex_(0)
    , speedAccumulator_(0.0f)
{
}

MissionStartContext MissionRuntime::startMission(const MissionDefinition& mission) {
    std::cout << "[MissionRuntime] Iniciando misión: " << mission.name << std::endl;

    // Guardar misión
    currentMission_ = mission;
    hasMission_ = true;
    phase_ = MissionPhase::Briefing;
    menuExitRequested_ = false;

    // Inicializar waypoints
    activeWaypointIndex_ = 0;
    waypointsCaptured_.clear();
    waypointsCaptured_.resize(mission.waypoints.size(), false);

    // Resetear métricas
    metrics_ = MissionMetrics();
    metrics_.totalWaypoints = static_cast<int>(mission.waypoints.size());
    missionStartTime_ = std::chrono::steady_clock::now();
    speedAccumulator_ = 0.0f;

    // Crear contexto de inicio
    MissionStartContext context;
    context.countdownSeconds = 3;
    context.showBriefing = !mission.briefing.empty();
    context.startPosition = mission.startPosition;
    context.startOrientation = mission.startOrientation;
    context.recommendedSpeed = 150.0f; // kt - velocidad de crucero típica
    context.recommendedAltitude = mission.startPosition.y * 3.28084f; // m -> ft

    std::cout << "[MissionRuntime] Briefing preparado, esperando confirmación del piloto" << std::endl;

    return context;
}

void MissionRuntime::confirmReadyToFly() {
    if (phase_ == MissionPhase::Briefing) {
        phase_ = MissionPhase::InProgress;
        missionStartTime_ = std::chrono::steady_clock::now(); // Resetear cronómetro
        std::cout << "[MissionRuntime] Misión iniciada - Buena suerte, piloto!" << std::endl;
    }
}

void MissionRuntime::markWaypointCaptured(int waypointIndex) {
    if (waypointIndex < 0 || waypointIndex >= static_cast<int>(waypointsCaptured_.size())) {
        return;
    }

    if (!waypointsCaptured_[waypointIndex]) {
        waypointsCaptured_[waypointIndex] = true;
        metrics_.waypointsCaptured++;

        std::cout << "[MissionRuntime] Waypoint " << (waypointIndex + 1) << " capturado ("
                  << metrics_.waypointsCaptured << "/" << metrics_.totalWaypoints << ")" << std::endl;

        // Avanzar al siguiente waypoint
        if (waypointIndex == activeWaypointIndex_) {
            activeWaypointIndex_++;
        }

        // Verificar si se completó la misión
        if (metrics_.waypointsCaptured >= metrics_.totalWaypoints) {
            markCompletion();
        }
    }
}

void MissionRuntime::markCompletion() {
    if (phase_ != MissionPhase::Completed && phase_ != MissionPhase::FreeFlight) {
        phase_ = MissionPhase::Completed;

        // Calcular tiempo total
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - missionStartTime_);
        metrics_.totalTimeSeconds = static_cast<float>(duration.count());

        // El mensaje de completación lo muestra MissionOverlay::showCompletionPrompt()
        std::cout << "[MissionRuntime] Misión completada - Esperando decisión del piloto" << std::endl;
    }
}

void MissionRuntime::continueFreeFlight() {
    if (phase_ == MissionPhase::Completed) {
        phase_ = MissionPhase::FreeFlight;
        std::cout << "[MissionRuntime] Modo vuelo libre activado - Disfruta del cielo!" << std::endl;
    }
}

void MissionRuntime::requestMenuExit() {
    menuExitRequested_ = true;
    std::cout << "[MissionRuntime] Retorno al menú solicitado" << std::endl;
}

void MissionRuntime::reset() {
    std::cout << "[MissionRuntime] Reiniciando runtime" << std::endl;

    phase_ = MissionPhase::Briefing;
    hasMission_ = false;
    menuExitRequested_ = false;
    activeWaypointIndex_ = 0;
    waypointsCaptured_.clear();

    metrics_ = MissionMetrics();
    speedAccumulator_ = 0.0f;
}

void MissionRuntime::updateProgress(const flight::FlightData& flightData, float dt) {
    if (!hasMission_ || phase_ != MissionPhase::InProgress) {
        return;
    }

    // Verificar captura de waypoint activo
    if (activeWaypointIndex_ < static_cast<int>(currentMission_.waypoints.size())) {
        const auto& waypoint = currentMission_.waypoints[activeWaypointIndex_];
        if (checkWaypointCapture(flightData.position, waypoint.position)) {
            markWaypointCaptured(activeWaypointIndex_);
        }
    }
}

void MissionRuntime::updateMetrics(const flight::FlightData& flightData, float dt) {
    if (!hasMission_ || (phase_ != MissionPhase::InProgress && phase_ != MissionPhase::FreeFlight)) {
        return;
    }

    // Actualizar velocidad promedio
    speedAccumulator_ += flightData.airspeed * dt;
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - missionStartTime_);
    float totalTime = duration.count() / 1000.0f;
    
    if (totalTime > 0.0f) {
        metrics_.averageSpeed = speedAccumulator_ / totalTime;
    }

    // Actualizar altitud máxima
    if (flightData.altitude > metrics_.maxAltitude) {
        metrics_.maxAltitude = flightData.altitude;
    }
}

bool MissionRuntime::checkWaypointCapture(const glm::vec3& planePos, const glm::vec3& waypointPos, float captureRadius) {
    float distance = glm::distance(planePos, waypointPos);
    return distance < captureRadius;
}

} // namespace mission
