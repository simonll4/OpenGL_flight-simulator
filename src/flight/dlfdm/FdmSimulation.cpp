#include "flight/dlfdm/FdmSimulation.h"

#include <cmath>

#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

namespace flight
{
    namespace
    {
        constexpr float kRadToDeg = 180.0f / glm::pi<float>();
        constexpr float kMpsToKt = 1.94384449f;     // 1 m/s -> kt
        constexpr float kMpsToFpm = 196.850394f;    // 1 m/s -> ft/min
        constexpr float kMetersToFeet = 3.2808399f; // 1 m -> ft
    }

    FdmSimulation::FdmSimulation()
        : fixedTimeStep_(1.0f / 120.0f), accumulator_(0.0f)
    {
    }

    void FdmSimulation::initialize()
    {
        aircraftParams_ = buildDefaultAircraft();
        trimState_ = buildDefaultTrimState();
        trimControls_ = buildDefaultTrimControls();

        solver_ = std::make_unique<dlfdm::FDMSolver>(aircraftParams_, fixedTimeStep_);
        solver_->setState(trimState_);

        controls_ = trimControls_;
        accumulator_ = 0.0f;

        syncState();
    }

    void FdmSimulation::setNormalizedInputs(float elevator, float aileron, float rudder, float throttle)
    {
        const float elevatorAuthority = glm::radians(8.0f);
        const float aileronAuthority = glm::radians(12.0f);

        // Elevator -----------------------------------------------------------
        float commandedElevator = trimControls_.elevator + elevator * elevatorAuthority;
        controls_.elevator = glm::clamp(commandedElevator,
                                        aircraftParams_.min_elevator,
                                        aircraftParams_.max_elevator);

        // Aileron - FDM maneja damping automáticamente
        float commandedAileron = trimControls_.aileron + aileron * aileronAuthority;
        controls_.aileron = glm::clamp(commandedAileron,
                                       aircraftParams_.min_aileron,
                                       aircraftParams_.max_aileron);

        // Rudder -------------------------------------------------------------
        float commandedRudder = trimControls_.rudder + rudder * aircraftParams_.max_rudder;
        controls_.rudder = glm::clamp(commandedRudder,
                                      -aircraftParams_.max_rudder,
                                      aircraftParams_.max_rudder);

        // Throttle -----------------------------------------------------------
        controls_.throttle = glm::clamp(throttle, 0.0f, 1.0f);
    }

    void FdmSimulation::enableLogging(const std::string &filename)
    {
        logFile_.open(filename, std::ios::app);
        if (logFile_.is_open())
        {
            solver_->log_titles(logFile_);
        }
    }

    void FdmSimulation::disableLogging()
    {
        if (logFile_.is_open())
        {
            logFile_.close();
        }
    }

    void FdmSimulation::writeLogEntry()
    {
        if (logFile_.is_open())
        {
            solver_->log_state(logFile_);
        }
    }

    void FdmSimulation::update(float deltaTime)
    {
        if (!solver_)
        {
            return;
        }

        accumulator_ += deltaTime;
        while (accumulator_ >= fixedTimeStep_)
        {
            validatePhysicalState();
            solver_->update(controls_);

            if (logFile_.is_open())
            {
                writeLogEntry();
            }

            accumulator_ -= fixedTimeStep_;
        }

        syncState();
    }

    void FdmSimulation::validatePhysicalState()
    {
        if (!solver_)
            return;

        const dlfdm::AircraftState &state = solver_->getState();
        float airspeed = glm::length(state.boby_velocity);
        float gForce = solver_->getGForce();

        // Stall detection
        stateValidation_.isStalled = (airspeed < MIN_AIRSPEED);
        stateValidation_.stallMargin = glm::clamp(airspeed / MIN_AIRSPEED, 0.0f, 2.0f);

        // G-force limits
        stateValidation_.gForceMargin = glm::clamp(MAX_G_FORCE / gForce, 0.0f, 2.0f);

        // Pitch limits
        // Spin detection
        float rollRate = state.body_omega.x;
        float yawRate = state.body_omega.z;
        float spinIndicator = std::abs(rollRate) + std::abs(yawRate);
        stateValidation_.isSpinning = (spinIndicator > glm::radians(180.0f));
    }

    void FdmSimulation::syncState()
    {
        const dlfdm::AircraftState &state = solver_->getState();
        const auto derivatives = solver_->get_state_dot();
        const glm::mat3 bodyToNed = buildBodyToNed(state);

        worldPosition_ = nedToWorld(state.intertial_position);
        worldOrientation_ = bodyToWorld(bodyToNed);
        trueAirspeed_ = glm::length(state.boby_velocity);

        updateFlightData(state, derivatives, bodyToNed);
    }

    void FdmSimulation::updateFlightData(const dlfdm::AircraftState &state,
                                         const dlfdm::AircraftDynamics::StateDerivatives &derivatives,
                                         const glm::mat3 &bodyToNed)
    {
        const float altitudeMeters = -state.intertial_position.z;
        cachedFlightData_.altitude = altitudeMeters * kMetersToFeet;

        cachedFlightData_.airspeed = trueAirspeed_ * kMpsToKt;
        cachedFlightData_.verticalSpeed = -derivatives.ned_position_dot.z * kMpsToFpm;

        cachedFlightData_.position = worldPosition_;
        const glm::vec3 nedVelocity = bodyToNed * state.boby_velocity;
        cachedFlightData_.velocity = nedToWorld(nedVelocity);

        cachedFlightData_.cameraFront = worldOrientation_ * glm::vec3(0.0f, 0.0f, -1.0f);
        cachedFlightData_.cameraUp = worldOrientation_ * glm::vec3(0.0f, 1.0f, 0.0f);
        cachedFlightData_.cameraRight = worldOrientation_ * glm::vec3(1.0f, 0.0f, 0.0f);

        const glm::vec3 euler = glm::eulerAngles(worldOrientation_);
        // Corrección de signos para orientación correcta
        // euler.x = +pitch físico, euler.z = -roll físico (debido a bodyToWorld swap)
        cachedFlightData_.pitch = glm::degrees(euler.x); // Sin inversión: +climb = +pitch
        cachedFlightData_.roll = -glm::degrees(euler.z); // Con inversión: +bank right = +roll

        const glm::vec3 front = cachedFlightData_.cameraFront;
        float heading = glm::degrees(std::atan2(front.x, -front.z));
        heading = std::fmod(heading, 360.0f);
        if (heading < 0.0f)
            heading += 360.0f;

        cachedFlightData_.heading = heading;
        cachedFlightData_.yaw = heading;

        // Datos aerodinámicos
        cachedFlightData_.angleOfAttack = solver_->getAngleOfAttack();
        cachedFlightData_.sideslip = solver_->getSideslip();
        cachedFlightData_.dynamicPressure = solver_->getDynamicPressure();

        // Velocidades angulares
        cachedFlightData_.rollRate = state.body_omega.x;
        cachedFlightData_.pitchRate = state.body_omega.y;
        cachedFlightData_.yawRate = state.body_omega.z;

        // G-forces
        cachedFlightData_.gForce = solver_->getGForce();
        glm::vec3 gComps = solver_->getGForceComponents();
        cachedFlightData_.gForceNormal = gComps.z;
        cachedFlightData_.gForceLateral = gComps.y;
        cachedFlightData_.loadFactor = glm::length(gComps);
    }

    dlfdm::AircraftParameters FdmSimulation::buildDefaultAircraft() const
    {
        dlfdm::AircraftParameters p;
        p.mass = 1815.0f;
        p.Ixx = 1084.6f;
        p.Iyy = 6507.9f;
        p.Izz = 7050.2f;
        p.Ixz = 271.16f;

        p.wingArea = 12.63f;
        p.wingChord = 1.64f;
        p.wingSpan = 8.01f;

        p.maxThrust = 11120.0f;

        p.CL0 = 0.15f;
        p.CLa = 5.5f;
        p.CL_delta_e = 0.38f;
        p.CD0 = 0.0205f;
        p.CDa = 0.12f;
        p.Cm0 = -0.08f;
        p.Cma = -0.24f;
        p.Cm_q = -15.7f;
        p.CY_beta = -1.0f;
        p.CY_r = 0.61f;
        p.CY_delta_r = 0.028f;
        p.Cl_beta = -0.11f;
        p.Cl_p = -0.39f;
        p.Cl_r = 0.28f;
        p.Cn_beta = 0.17f;
        p.Cn_p = 0.09f;
        p.Cn_r = -0.26f;

        p.Cm_delta_e = -0.88f;
        p.Cl_delta_a = 0.10f;
        p.Cn_delta_r = -0.12f;

        p.min_elevator = glm::radians(-15.0f);
        p.max_elevator = glm::radians(20.0f);
        p.min_aileron = glm::radians(-20.0f);
        p.max_aileron = glm::radians(20.0f);
        p.max_rudder = glm::radians(20.0f);

        return p;
    }

    dlfdm::AircraftState FdmSimulation::buildDefaultTrimState() const
    {
        dlfdm::AircraftState state;
        state.intertial_position = glm::vec3(0.0f, 0.0f, -1500.0f);
        state.boby_velocity = glm::vec3(149.998f, 0.0f, -0.36675f);
        state.body_omega = glm::vec3(0.0f);
        state.phi = 0.0f;
        state.theta = 0.0f;
        state.psi = 0.0f;
        return state;
    }

    dlfdm::ControlInputs FdmSimulation::buildDefaultTrimControls() const
    {
        dlfdm::ControlInputs controls;
        controls.elevator = -0.09024f;
        controls.aileron = 0.0f;
        controls.rudder = 0.0f;
        controls.throttle = 0.3202f;
        return controls;
    }

    glm::vec3 FdmSimulation::nedToWorld(const glm::vec3 &ned) const
    {
        return glm::vec3(ned.y, -ned.z, -ned.x);
    }

    glm::mat3 FdmSimulation::buildBodyToNed(const dlfdm::AircraftState &state) const
    {
        const float cp = glm::cos(state.phi);
        const float sp = glm::sin(state.phi);
        const float ct = glm::cos(state.theta);
        const float st = glm::sin(state.theta);
        const float cy = glm::cos(state.psi);
        const float sy = glm::sin(state.psi);

        glm::mat3 bodyToNed(
            ct * cy, ct * sy, -st,
            sp * st * cy - cp * sy, sp * st * sy + cp * cy, sp * ct,
            cp * st * cy + sp * sy, cp * st * sy - sp * cy, cp * ct);

        return bodyToNed;
    }

    glm::quat FdmSimulation::bodyToWorld(const glm::mat3 &bodyToNed) const
    {
        const glm::vec3 col0(0.0f, 0.0f, -1.0f);
        const glm::vec3 col1(1.0f, 0.0f, 0.0f);
        const glm::vec3 col2(0.0f, -1.0f, 0.0f);
        const glm::mat3 nedToWorldMat(col0, col1, col2);
        const glm::mat3 worldToNedMat = glm::transpose(nedToWorldMat);

        const glm::mat3 bodyToWorld = nedToWorldMat * bodyToNed * worldToNedMat;
        return glm::normalize(glm::quat_cast(glm::mat4(bodyToWorld)));
    }

} // namespace flight
