// flight/data/FlightData.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace flight
{

    /**
     * FlightData: datos que el HUD necesita para "instrumentos"
     *
     * =============================================================================
     * CONVENCIONES DE UNIDADES (ESTÁNDARES AERONÁUTICOS):
     * =============================================================================
     * 
     * NAVEGACIÓN Y VELOCIDAD:
     * - airspeed:      NUDOS (kt) - Estándar mundial aviación
     * - altitude:      PIES (ft) - Estándar mundial aviación
     * - verticalSpeed: PIES/MINUTO (ft/min) - Estándar aviación
     *                  (HUD muestra ft/min/100 para compactar: 4000→"40")
     * 
     * ACTITUD (ángulos en GRADOS):
     * - pitch:   Cabeceo (−90° a +90°, positivo = nariz arriba)
     * - roll:    Alabeo (−180° a +180°, positivo = ala derecha abajo)
     * - heading: Rumbo (0° a 360°, 0°=Norte, 90°=Este)
     * - yaw:     Guiñada (asumimos sin derrape → yaw = heading)
     *
     * MUNDO (sistema interno de simulación):
     * - position: metros (m)
     * - velocity: metros/segundo (m/s)
     * 
     * DATOS AERODINÁMICOS:
     * - angleOfAttack: radianes [rad]
     * - sideslip: radianes [rad]
     * - dynamicPressure: Pascales [Pa]
     * - rollRate/pitchRate/yawRate: radianes/segundo [rad/s]
     * - gForce/loadFactor: adimensional [G]
     *
     * =============================================================================
     * CONSISTENCIA DE INSTRUMENTOS EN HUD:
     * =============================================================================
     * 1. SpeedIndicator:   muestra kt directamente (ej: "250")
     * 2. Altimeter:        muestra ft directamente (ej: "3400")
     * 3. VSI:              muestra ft/min/100 (ej: "40" = 4000 ft/min)
     * 
     * Esta convención es estándar en HUDs militares (F-16, F-18, Gripen, Rafale)
     *
     * =============================================================================
     * CONSUMIDORES CLAVE DEL STRUCT:
     * =============================================================================
     * - `hud::FlightHUD` y sus instrumentos leen directamente `airspeed`, `altitude`,
     *   `verticalSpeed`, `pitch`, `roll`, `heading`, `waypointBearing` y `waypointDistance`.
     * - `mission::MissionRuntime` usa `position` (m) para capturar waypoints y
     *   `airspeed`/`altitude` para métricas agregadas (kt/ft respectivamente).
     * - `gfx::WaypointRenderer`/`WaypointIndicator` emplean `position`,
     *   `targetWaypoint`, `waypointDistance` y `waypointBearing` en metros/grados.
     * - Los datos aerodinámicos (`angleOfAttack`, `sideslip`, `dynamicPressure`,
     *   `rollRate/pitchRate/yawRate`) se almacenan en las unidades nativas del solver
     *   (rad, Pa, rad/s) para futuros sistemas (protecciones, flight director, etc.).
     * =============================================================================
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

        // Fuerzas
        float gForce = 1.0f;            // G-forces totales
        float gForceNormal = 1.0f;      // G normal (vertical)
        float gForceLateral = 0.0f;     // G lateral (horizontal)
        float loadFactor = 1.0f;        // n = G / g

        // Sistema de Waypoints (navegación)
        glm::vec3 targetWaypoint = glm::vec3(0.0f);
        bool hasActiveWaypoint = false;
        float waypointDistance = 0.0f;
        float waypointBearing = 0.0f;

        // Actualización de datos de vuelo desde cámara
        void updateFromCamera(const glm::vec3 &front,
                              const glm::vec3 &up,
                              const glm::vec3 &pos,
                              float deltaTime);
    };

} // namespace flight
