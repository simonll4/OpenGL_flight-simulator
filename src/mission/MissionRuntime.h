/**
 * @file MissionRuntime.h
 * @brief Gestión del estado de ejecución de una misión
 *
 * Este módulo centraliza toda la lógica de estado de una sesión de misión:
 * - Fases: Briefing, InProgress, Completed, FreeFlight
 * - Control de waypoints y progreso
 * - Métricas de rendimiento
 */

#pragma once

#include "MissionDefinition.h"
#include <chrono>
#include <string>

namespace flight
{
    struct FlightData;
}

namespace mission
{

    /**
     * @brief Fases de una misión
     */
    enum class MissionPhase
    {
        Briefing,   // Mostrando briefing inicial, físicas pausadas
        InProgress, // Misión en curso, físicas activas
        Completed,  // Misión completada, esperando decisión del piloto
        FreeFlight  // Vuelo libre post-misión, sin waypoints
    };

    /**
     * @brief Contexto de inicio de misión
     */
    struct MissionStartContext
    {
        int countdownSeconds = 3;
        bool showBriefing = true;
        glm::vec3 startPosition;
        glm::quat startOrientation;
        float recommendedSpeed = 150.0f;     // kt
        float recommendedAltitude = 1500.0f; // ft
    };

    /**
     * @brief Métricas de rendimiento de la misión
     */
    struct MissionMetrics
    {
        float totalTimeSeconds = 0.0f;
        int waypointsCaptured = 0;
        int totalWaypoints = 0;
        float averageSpeed = 0.0f; // kt
        float maxAltitude = 0.0f;  // ft
        bool perfectRun = true;    // Sin errores graves
    };

    /**
     * @brief Gestor del estado de ejecución de misiones
     *
     * Centraliza la información que otros subsistemas necesitan:
     * - HUD y overlay preguntan la fase/flags para decidir qué mostrar
     * - WaypointSystem consulta el índice actual y captura
     * - Menús utilizan métricas para resúmenes de fin de misión
     */
    class MissionRuntime
    {
    public:
        MissionRuntime();
        ~MissionRuntime() = default;

        // ========================================================================
        // CONTROL DE MISIÓN
        // ========================================================================

        /**
         * @brief Inicia una nueva misión
         * @param mission Definición de la misión a ejecutar
         * @return Contexto de inicio con configuración inicial
         */
        MissionStartContext startMission(const MissionDefinition &mission);

        /**
         * @brief Marca un waypoint como capturado
         * @param waypointIndex Índice del waypoint capturado
         */
        void markWaypointCaptured(int waypointIndex);

        /**
         * @brief Marca la misión como completada (todos los waypoints capturados)
         */
        void markCompletion();

        /**
         * @brief Continúa en modo vuelo libre después de completar la misión
         */
        void continueFreeFlight();

        /**
         * @brief Solicita retorno al menú principal
         */
        void requestMenuExit();

        /**
         * @brief Reinicia el runtime para una nueva sesión
         */
        void reset();

        // ========================================================================
        // ACTUALIZACIÓN DE PROGRESO
        // ========================================================================

        /**
         * @brief Actualiza el progreso de la misión basado en datos de vuelo
         * @param flightData Datos actuales de vuelo
         * @param dt Delta time del frame
         */
        void updateProgress(const flight::FlightData &flightData, float dt);

        /**
         * @brief Actualiza métricas de rendimiento
         * @param flightData Datos actuales de vuelo
         * @param dt Delta time del frame
         */
        void updateMetrics(const flight::FlightData &flightData, float dt);

        // ========================================================================
        // CONSULTAS DE ESTADO
        // ========================================================================

        /**
         * @brief Obtiene la fase actual de la misión
         */
        MissionPhase phase() const { return phase_; }

        /**
         * @brief Verifica si hay una misión activa
         */
        bool hasMission() const { return hasMission_; }

        /**
         * @brief Verifica si la misión está completada
         */
        bool isCompleted() const { return phase_ == MissionPhase::Completed || phase_ == MissionPhase::FreeFlight; }

        /**
         * @brief Verifica si el sistema de waypoints está habilitado
         */
        bool areWaypointsEnabled() const
        {
            return hasMission_ && (phase_ == MissionPhase::InProgress || phase_ == MissionPhase::Briefing);
        }

        /**
         * @brief Verifica si las físicas deben ejecutarse
         */
        bool shouldRunPhysics() const
        {
            return phase_ == MissionPhase::InProgress || phase_ == MissionPhase::FreeFlight;
        }

        /**
         * @brief Verifica si se debe mostrar el overlay
         */
        bool shouldShowOverlay() const
        {
            return phase_ == MissionPhase::Briefing || phase_ == MissionPhase::Completed;
        }

        /**
         * @brief Verifica si se solicitó retorno al menú
         */
        bool menuExitRequested() const { return menuExitRequested_; }

        /**
         * @brief Obtiene el waypoint activo actual
         */
        int getActiveWaypointIndex() const { return activeWaypointIndex_; }

        /**
         * @brief Obtiene la misión actual
         */
        const MissionDefinition &getMission() const { return currentMission_; }

        /**
         * @brief Obtiene las métricas de rendimiento
         */
        const MissionMetrics &getMetrics() const { return metrics_; }

        /**
         * @brief Confirma que el piloto está listo para volar (sale del briefing)
         */
        void confirmReadyToFly();

    private:
        // Estado de la misión
        MissionPhase phase_;
        bool hasMission_;
        MissionDefinition currentMission_;
        bool menuExitRequested_;

        // Waypoints y progreso
        int activeWaypointIndex_;
        std::vector<bool> waypointsCaptured_;

        // Métricas acumuladas
        MissionMetrics metrics_;
        std::chrono::steady_clock::time_point missionStartTime_;
        float speedAccumulator_; // Para calcular velocidad promedio

        // Helpers
        /**
         * @brief Evalúa si el avión está dentro del radio de captura del waypoint.
         */
        bool checkWaypointCapture(const glm::vec3 &planePos, const glm::vec3 &waypointPos, float captureRadius = 100.0f);
    };

} // namespace mission
