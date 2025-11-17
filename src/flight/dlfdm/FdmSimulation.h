#pragma once

#include <memory>
#include <fstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "flight/data/FlightData.h"
#include <dlfdm/fdmsolver.h>

namespace flight
{
    /**
     * @brief Wrapper de alto nivel para integrar el FDMSolver dentro del simulador.
     *
     * Gestiona el tiempo fijo del integrador, mappea entradas normalizadas [-1,1]
     * a deflexiones reales y expone posición/orientación listas para el motor 3D.
     */
    class FdmSimulation
    {
    public:
        FdmSimulation();

        void initialize();

        struct StateValidation
        {
            bool isStalled = false;
            bool isSpinning = false;
            float stallMargin = 1.0f;
            float gForceMargin = 1.0f;
        };

        void setNormalizedInputs(float elevator, float aileron, float rudder, float throttle);
        void validatePhysicalState();
        StateValidation getStateValidation() const { return stateValidation_; }

        void enableLogging(const std::string &filename);
        void disableLogging();
        bool isLoggingEnabled() const { return logFile_.is_open(); }

        /**
         * @brief Actualiza el solver usando un acumulador de paso fijo.
         */
        void update(float deltaTime);

        glm::vec3 getWorldPosition() const { return worldPosition_; }
        glm::quat getWorldOrientation() const { return worldOrientation_; }
        float getTrueAirspeed() const { return trueAirspeed_; }
        const flight::FlightData &getFlightData() const { return cachedFlightData_; }

    private:
        dlfdm::AircraftParameters aircraftParams_{};
        dlfdm::AircraftState trimState_{};
        dlfdm::ControlInputs trimControls_{};
        std::unique_ptr<dlfdm::FDMSolver> solver_;
        dlfdm::ControlInputs controls_{};

        float fixedTimeStep_;
        float accumulator_;

        glm::vec3 worldPosition_{0.0f};
        glm::quat worldOrientation_{1.0f, 0.0f, 0.0f, 0.0f};
        float trueAirspeed_ = 0.0f;
        flight::FlightData cachedFlightData_;
        StateValidation stateValidation_;
        std::ofstream logFile_;

        static constexpr float MIN_AIRSPEED = 15.0f;
        static constexpr float MAX_G_FORCE = 9.0f;

        void syncState();
        void writeLogEntry();
        void updateFlightData(const dlfdm::AircraftState &state,
                              const dlfdm::AircraftDynamics::StateDerivatives &derivatives,
                              const glm::mat3 &bodyToNed);

        dlfdm::AircraftParameters buildDefaultAircraft() const;
        dlfdm::AircraftState buildDefaultTrimState() const;
        dlfdm::ControlInputs buildDefaultTrimControls() const;

        glm::vec3 nedToWorld(const glm::vec3 &ned) const;
        glm::mat3 buildBodyToNed(const dlfdm::AircraftState &state) const;
        glm::quat bodyToWorld(const glm::mat3 &bodyToNed) const;
    };

} // namespace flight
