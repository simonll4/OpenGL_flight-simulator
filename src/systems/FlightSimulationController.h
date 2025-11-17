#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "flight/data/FlightData.h"
#include "flight/dlfdm/FdmSimulation.h"

struct GLFWwindow;

namespace systems
{

    /**
     * @brief Adaptador entre el solver de vuelo (DLFDM) y el resto del simulador.
     *
     * Lee input de teclado, lo filtra para evitar movimientos bruscos y alimenta
     * al `flight::FdmSimulation`. Expone posición/orientación/telemetría para el HUD.
     */
    class FlightSimulationController
    {
    public:
        FlightSimulationController();

        /// Inicializa estado interno y sincroniza la simulación.
        void initialize();
        /// Reinicia la aeronave para una misión recién cargada.
        void resetForMission();
        /// Lee teclas por frame y actualiza el joystick virtual.
        void handleControls(GLFWwindow *window, float dt);
        /// Avanza el solver un paso y actualiza FlightData.
        void step(float dt);

        /// Acceso de solo lectura a la telemetría calculada.
        const flight::FlightData &getFlightData() const { return flightData_; }
        /// Acceso mutable (para sistemas que requieran modificarla).
        flight::FlightData &getFlightData() { return flightData_; }
        /// Posición del avión en el mundo.
        const glm::vec3 &planePosition() const { return planePos_; }
        /// Orientación (cuaternión) del avión.
        const glm::quat &planeOrientation() const { return planeOrientation_; }
        /// Velocidad verdadera (kt convertidos desde m/s).
        float planeSpeed() const { return planeSpeed_; }
        /// Último comando de potencia normalizado [0,1].
        float throttleInput() const { return throttleInput_; }

        /// Fuerza el valor de throttle desde otro sistema (autopiloto, scripts).
        void setThrottle(float value);

    private:
        /// Representa el estado de un joystick virtual en rangos [-1,1].
        struct VirtualJoystick
        {
            float aileron = 0.0f;  ///< Entrada de alabeo (roll).
            float elevator = 0.0f; ///< Entrada de cabeceo (pitch).
            float rudder = 0.0f;   ///< Entrada de guiñada (yaw).
        };

        /// Lleva gradualmente un control hacia el centro (0) con una velocidad dada.
        float centerControl(float value, float factor, float dt);
        /// Aplica un desplazamiento hacia una dirección (-1/+1) respetando límites.
        float moveControl(float value, float direction, float factor, float dt);
        /// Filtro exponencial para emular respuesta suave de los actuadores.
        float smoothControlTowards(float current, float target, float responseRate, float dt);

        flight::FdmSimulation simulation_; ///< Solver físico-propulsivo del avión.
        flight::FlightData flightData_;    ///< Última telemetría publicada.

        glm::vec3 planePos_ = glm::vec3(0.0f, 1500.0f, 0.0f);            ///< Posición inicial elevada.
        glm::quat planeOrientation_ = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); ///< Orientación neutra.
        float planeSpeed_ = 0.0f;                                        ///< Magnitud de velocidad en m/s convertida a kt en HUD.

        VirtualJoystick joystick_{};         ///< Lectura directa del teclado.
        VirtualJoystick filteredJoystick_{}; ///< Señal suavizada que llega al FDM.

        glm::vec3 controlFactor_ = glm::vec3(3.0f, 1.0f, 3.0f); ///< Sensibilidad por eje.

        /// Tiempos de respuesta para cada canal (mayor valor = reacciona más rápido).
        struct ControlResponseRates
        {
            float roll;
            float pitch;
            float yaw;
            float throttle;
        } responseRates_{8.0f, 6.0f, 4.0f, 3.0f};

        float throttleInput_ = 0.32f;             ///< Potencia normalizada comandada.
        float filteredThrottle_ = throttleInput_; ///< Potencia filtrada que recibe el FDM.
    };

} // namespace systems
