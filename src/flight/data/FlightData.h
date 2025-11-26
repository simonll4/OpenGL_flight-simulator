/**
 * @file FlightData.h
 * @brief Flight data structure with aeronautical units and conventions.
 */

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
        // --- Actitud (Attitude) ---
        float pitch = 0.0f; ///< Pitch angle in degrees [-90, 90]. Positive is nose up.
        float roll = 0.0f;  ///< Roll angle in degrees [-180, 180]. Positive is right wing down.
        float yaw = 0.0f;   ///< Yaw angle in degrees [0, 360).

        // --- Navegación (Navigation) ---
        float heading = 0.0f;       ///< Heading in degrees [0, 360). 0 is North, 90 is East.
        float airspeed = 0.0f;      ///< Airspeed in knots (kt).
        float altitude = 1000.0f;   ///< Altitude in feet (ft).
        float verticalSpeed = 0.0f; ///< Vertical speed in feet per minute (ft/min).

        // --- Estado en mundo (World State) ---
        glm::vec3 position = glm::vec3(0.0f, 304.8f, 0.0f); ///< Position in world coordinates (meters).
        glm::vec3 velocity = glm::vec3(0.0f);               ///< Velocity vector in world coordinates (m/s).

        // --- Base de cámara ortonormal (Camera Basis) ---
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); ///< Camera forward vector.
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     ///< Camera up vector.
        glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);  ///< Camera right vector.

        // --- Datos aerodinámicos (Aerodynamics) ---
        float angleOfAttack = 0.0f;   ///< Angle of attack (alpha) in radians.
        float sideslip = 0.0f;        ///< Sideslip angle (beta) in radians.
        float dynamicPressure = 0.0f; ///< Dynamic pressure (qbar) in Pascals.

        // --- Velocidades angulares body frame (Angular Rates) ---
        float rollRate = 0.0f;  ///< Roll rate (p) in rad/s.
        float pitchRate = 0.0f; ///< Pitch rate (q) in rad/s.
        float yawRate = 0.0f;   ///< Yaw rate (r) in rad/s.

        // --- Fuerzas (Forces) ---
        float gForce = 1.0f;        ///< Total G-force magnitude.
        float gForceNormal = 1.0f;  ///< Normal (vertical) G-force.
        float gForceLateral = 0.0f; ///< Lateral G-force.
        float loadFactor = 1.0f;    ///< Load factor (n = G / g).

        // --- Sistema de Waypoints (Navigation System) ---
        glm::vec3 targetWaypoint = glm::vec3(0.0f); ///< Target waypoint position in world coordinates.
        bool hasActiveWaypoint = false;             ///< True if a waypoint is currently active.
        float waypointDistance = 0.0f;              ///< Distance to the active waypoint (meters).
        float waypointBearing = 0.0f;               ///< Bearing to the active waypoint (degrees).

        /**
         * @brief Updates flight data based on camera position and orientation.
         *
         * Calculates attitude (pitch, roll, heading) and flight parameters (airspeed, altitude, etc.)
         * from the camera's transformation matrix and movement.
         *
         * @param front Camera forward vector.
         * @param up Camera up vector.
         * @param pos Camera position in world space.
         * @param deltaTime Time elapsed since last update (seconds).
         */
        void updateFromCamera(const glm::vec3 &front,
                              const glm::vec3 &up,
                              const glm::vec3 &pos,
                              float deltaTime);
    };

} // namespace flight
