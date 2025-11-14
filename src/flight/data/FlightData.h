// flight/data/FlightData.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace flight
{

    /**
     * FlightData: datos que el HUD necesita para “instrumentos”
     *
     * Convenciones de unidades:
     * - Ángulos: grados.
     * - position/velocity: metros y m/s (mundo).
     * - airspeed: nudos (kt). verticalSpeed: pies/min (ft/min). altitude: pies (ft).
     *
     * Vocabulario llano:
     * - pitch (cabeceo): levantar/bajar la nariz (−90°..+90°).
     * - roll (alabeo): inclinar las alas (−180°..+180°, derecha positivo).
     * - heading (rumbo): hacia dónde “apunta” en planta (0° = norte, mirando −Z).
     * - yaw (guiñada): giro sobre vertical. Aquí asumimos sin derrape → yaw = heading.
     */
    struct FlightData
    {
        // Actitud
        float pitch = 0.0f;
        float roll = 0.0f;
        float yaw = 0.0f;

        // Navegación
        float heading = 0.0f;       // 0..360
        float airspeed = 0.0f;      // kt
        float altitude = 1000.0f;   // ft
        float verticalSpeed = 0.0f; // ft/min

        // Estado en mundo (m / m/s)
        glm::vec3 position = glm::vec3(0.0f, 304.8f, 0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);

        // Base de cámara ortonormal
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);

        // Datos aerodinámicos (NUEVO)
        float angleOfAttack = 0.0f;     // alpha [rad]
        float sideslip = 0.0f;          // beta [rad]
        float dynamicPressure = 0.0f;   // qbar [Pa]

        // Velocidades angulares body frame (NUEVO)
        float rollRate = 0.0f;          // p [rad/s]
        float pitchRate = 0.0f;         // q [rad/s]
        float yawRate = 0.0f;           // r [rad/s]

        // Fuerzas (NUEVO)
        float gForce = 1.0f;            // G-forces totales
        float gForceNormal = 1.0f;      // G normal (vertical)
        float gForceLateral = 0.0f;     // G lateral (horizontal)
        float loadFactor = 1.0f;        // n = G / g

        // "Feeling" de instrumentos (suavizados)
        float tauAttitude = 0.10f;
        float tauVelocity = 0.15f;
        float maxPlausibleSpeed = 300.0f;

        // Sistema de Waypoints (navegación)
        glm::vec3 targetWaypoint = glm::vec3(0.0f);
        bool hasActiveWaypoint = false;
        float waypointDistance = 0.0f;
        float waypointBearing = 0.0f;

        // Deriva instrumentos desde cámara
        void updateFromCamera(const glm::vec3 &front,
                              const glm::vec3 &up,
                              const glm::vec3 &pos,
                              float deltaTime);

        // En modo telemetría sólo acota valores (no integra dinámica)
        void simulatePhysics(float deltaTime);

        // Utilidad pública (legado)
        float normalizeAngle(float angle);
    };

} // namespace flight
