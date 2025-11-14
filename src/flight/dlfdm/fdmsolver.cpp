#include <dlfdm/fdmsolver.h>

namespace dlfdm {

static std::ostream& operator<<(std::ostream& os, const glm::vec2 v){
    os << v.x << "," << v.y;
    return os;
}

static std::ostream& operator<<(std::ostream& os, const glm::vec3 v){
    os << v.x << "," << v.y << "," << v.z;
    return os;
}

FDMSolver::FDMSolver(const AircraftParameters& p, float dt)
    : aircraft_data_(p), aerodynamics(p), dynamics(p), time_step_(dt), time_(0.0f)
{
    // Initialize state
    aircraft_state_.intertial_position = glm::vec3(0.0f);
    aircraft_state_.boby_velocity = glm::vec3(10.0f, 0.0f, 0.0f);  // Initial forward velocity
    aircraft_state_.phi = 0.0f;
    aircraft_state_.theta = 0.0f;
    aircraft_state_.psi = 0.0f;
    aircraft_state_.body_omega = glm::vec3(0.0f);
}

void FDMSolver::update(const ControlInputs &controls) {
    // Clamp controls
    ControlInputs clamped_controls = controls;

    clamped_controls.throttle   = glm::clamp(clamped_controls.throttle,
                                             0.0f,
                                             1.0f);
    clamped_controls.elevator   = glm::clamp(clamped_controls.elevator,
                                             aircraft_data_.min_elevator,
                                             aircraft_data_.max_elevator);
    clamped_controls.aileron    = glm::clamp(clamped_controls.aileron,
                                             aircraft_data_.min_aileron,
                                             aircraft_data_.max_aileron);
    clamped_controls.rudder     = glm::clamp(clamped_controls.rudder,
                                             -aircraft_data_.max_rudder,
                                             aircraft_data_.max_rudder);

    // Calculate aerodynamic forces and moments
    aero_fm_ = aerodynamics.calculate(aircraft_state_.boby_velocity,
                                      aircraft_state_.body_omega,
                                      clamped_controls);

    // TODO: move thrust calculation here

    // Compute state derivatives en el estado actual
    state_deriv_ = dynamics.compute_derivatives(aircraft_state_, aero_fm_, clamped_controls);

    // Actualizar datos aerodinámicos derivados (alpha, beta, q, G, etc.)
    calculateAerodynamicData();
    calculateGForces();

    // Avanzar tiempo de simulación
    time_ += time_step_;

    // ================================================================
    // Integración RK4 (como en CGyAV-dlfdm)
    // ================================================================

    // k1
    AircraftState k1_state = aircraft_state_;
    AircraftDynamics::StateDerivatives k1_deriv = state_deriv_;

    // k2
    AircraftState k2_state = aircraft_state_;
    k2_state.intertial_position += k1_deriv.ned_position_dot * (time_step_ * 0.5f);
    k2_state.boby_velocity      += k1_deriv.body_velocity_dot * (time_step_ * 0.5f);
    k2_state.body_omega         += k1_deriv.body_omega_dot * (time_step_ * 0.5f);
    k2_state.phi                += k1_deriv.euler_dot.x * (time_step_ * 0.5f);
    k2_state.theta              += k1_deriv.euler_dot.y * (time_step_ * 0.5f);
    k2_state.psi                += k1_deriv.euler_dot.z * (time_step_ * 0.5f);

    aero_fm_ = aerodynamics.calculate(k2_state.boby_velocity,
                                      k2_state.body_omega,
                                      clamped_controls);
    auto k2_deriv = dynamics.compute_derivatives(k2_state, aero_fm_, clamped_controls);

    // k3
    AircraftState k3_state = aircraft_state_;
    k3_state.intertial_position += k2_deriv.ned_position_dot * (time_step_ * 0.5f);
    k3_state.boby_velocity      += k2_deriv.body_velocity_dot * (time_step_ * 0.5f);
    k3_state.body_omega         += k2_deriv.body_omega_dot * (time_step_ * 0.5f);
    k3_state.phi                += k2_deriv.euler_dot.x * (time_step_ * 0.5f);
    k3_state.theta              += k2_deriv.euler_dot.y * (time_step_ * 0.5f);
    k3_state.psi                += k2_deriv.euler_dot.z * (time_step_ * 0.5f);

    aero_fm_ = aerodynamics.calculate(k3_state.boby_velocity,
                                      k3_state.body_omega,
                                      clamped_controls);
    auto k3_deriv = dynamics.compute_derivatives(k3_state, aero_fm_, clamped_controls);

    // k4
    AircraftState k4_state = aircraft_state_;
    k4_state.intertial_position += k3_deriv.ned_position_dot * time_step_;
    k4_state.boby_velocity      += k3_deriv.body_velocity_dot * time_step_;
    k4_state.body_omega         += k3_deriv.body_omega_dot * time_step_;
    k4_state.phi                += k3_deriv.euler_dot.x * time_step_;
    k4_state.theta              += k3_deriv.euler_dot.y * time_step_;
    k4_state.psi                += k3_deriv.euler_dot.z * time_step_;

    aero_fm_ = aerodynamics.calculate(k4_state.boby_velocity,
                                      k4_state.body_omega,
                                      clamped_controls);
    auto k4_deriv = dynamics.compute_derivatives(k4_state, aero_fm_, clamped_controls);

    // Combinar RK4
    const float dt6 = time_step_ / 6.0f;

    aircraft_state_.intertial_position += (k1_deriv.ned_position_dot +
                                           2.0f * k2_deriv.ned_position_dot +
                                           2.0f * k3_deriv.ned_position_dot +
                                           k4_deriv.ned_position_dot) * dt6;

    aircraft_state_.boby_velocity += (k1_deriv.body_velocity_dot +
                                      2.0f * k2_deriv.body_velocity_dot +
                                      2.0f * k3_deriv.body_velocity_dot +
                                      k4_deriv.body_velocity_dot) * dt6;

    aircraft_state_.body_omega += (k1_deriv.body_omega_dot +
                                   2.0f * k2_deriv.body_omega_dot +
                                   2.0f * k3_deriv.body_omega_dot +
                                   k4_deriv.body_omega_dot) * dt6;

    aircraft_state_.phi += (k1_deriv.euler_dot.x +
                            2.0f * k2_deriv.euler_dot.x +
                            2.0f * k3_deriv.euler_dot.x +
                            k4_deriv.euler_dot.x) * dt6;

    aircraft_state_.theta += (k1_deriv.euler_dot.y +
                              2.0f * k2_deriv.euler_dot.y +
                              2.0f * k3_deriv.euler_dot.y +
                              k4_deriv.euler_dot.y) * dt6;

    aircraft_state_.psi += (k1_deriv.euler_dot.z +
                            2.0f * k2_deriv.euler_dot.z +
                            2.0f * k3_deriv.euler_dot.z +
                            k4_deriv.euler_dot.z) * dt6;

    // Clamp pitch to avoid singularities
    aircraft_state_.theta = glm::clamp(aircraft_state_.theta, -1.5f, 1.5f);

    // Normalize yaw to [-pi, pi]
    while (aircraft_state_.psi > 3.14159f) aircraft_state_.psi -= 6.28318f;
    while (aircraft_state_.psi < -3.14159f) aircraft_state_.psi += 6.28318f;
}

glm::mat4 FDMSolver::getModelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), aircraft_state_.intertial_position);
    model = glm::rotate(model, aircraft_state_.psi, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, aircraft_state_.theta, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, aircraft_state_.phi, glm::vec3(1.0f, 0.0f, 0.0f));
    return model;
}

void FDMSolver::log_titles(std::ostream &os, const char &sep) const
{
    os << "t [seg]" << sep;
    log_state_titles(os,sep);
    os << sep;
    aerodynamics.log_all_titles(os,sep);
    os << sep;
    dynamics.log_state_titles(os,sep);
    os << std::endl;
}

void FDMSolver::log_state(std::ostream& os, const char& sep) const {
    os << time_ << sep;
    log_aircraft_state(os,sep);
    os << sep;
    aerodynamics.log_all(os,sep);
    os << sep;
    dynamics.log_state_derivatives(os,sep);
    os << std::endl;
}

void FDMSolver::log_state_titles(std::ostream &os, const char &sep) const
{
    os << "x [m]" << sep << "y [m]" << sep << "z [m]" << sep;
    os << "phi [rad]" << sep << "theta [rad]" << sep << "psi [rad]" << sep;
    os << "u [m/s]" << sep << "v [m/s]" << sep << "w [m/s]" << sep;
    os << "p [rad/s]" << sep << "q [rad/s]" << sep << "r [rad/s]";
}

void FDMSolver::log_aircraft_state(std::ostream &os, const char &sep) const
{
    os << aircraft_state_.intertial_position << sep;
    os << aircraft_state_.phi << sep << aircraft_state_.theta << sep << aircraft_state_.psi << sep;
    os << aircraft_state_.boby_velocity << sep;
    os << aircraft_state_.body_omega;
}

void FDMSolver::calculateAerodynamicData() {
    aerodynamics.calculate_angles(aircraft_state_.boby_velocity, aero_angles_.x, aero_angles_.y);

    float V = glm::length(aircraft_state_.boby_velocity);
    const float rho = 1.225f;
    dynamic_pressure_ = 0.5f * rho * V * V;
}

void FDMSolver::calculateGForces() {
    const float g = 9.81f;
    glm::vec3 accel = state_deriv_.body_velocity_dot;

    g_force_ = glm::length(accel) / g;
    g_force_components_ = accel / g;
}

} // namespace dlfdm
