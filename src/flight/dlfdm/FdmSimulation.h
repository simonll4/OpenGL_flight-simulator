/**
 * @file FdmSimulation.h
 * @brief High-level FDM (Flight Dynamics Model) simulation wrapper.
 */

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
     * @brief High-level wrapper for integrating the FDMSolver into the simulator.
     *
     * Manages the fixed time step of the integrator, maps normalized inputs [-1, 1]
     * to real deflections, and exposes position/orientation ready for the 3D engine.
     */
    class FdmSimulation
    {
    public:
        FdmSimulation();

        /**
         * @brief Initializes the simulation with default aircraft parameters and trim state.
         */
        void initialize();

        struct StateValidation
        {
            bool isStalled = false;
            bool isSpinning = false;
            float stallMargin = 1.0f;
            float gForceMargin = 1.0f;
        };

        /**
         * @brief Sets the control inputs for the aircraft.
         *
         * @param elevator Normalized elevator input [-1, 1].
         * @param aileron Normalized aileron input [-1, 1].
         * @param rudder Normalized rudder input [-1, 1].
         * @param throttle Normalized throttle input [0, 1].
         */
        void setNormalizedInputs(float elevator, float aileron, float rudder, float throttle);

        /**
         * @brief Validates the current physical state (stall, spin, G-limits).
         */
        void validatePhysicalState();

        StateValidation getStateValidation() const { return stateValidation_; }

        void enableLogging(const std::string &filename);
        void disableLogging();
        bool isLoggingEnabled() const { return logFile_.is_open(); }

        /**
         * @brief Updates the solver using a fixed time step accumulator.
         *
         * @param deltaTime Time elapsed since the last frame (seconds).
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
