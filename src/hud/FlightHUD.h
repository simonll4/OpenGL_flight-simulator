#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../gfx/Renderer2D.h"
#include "../flight/FlightData.h"
#include "Instrument.h"

// Includes de instrumentos implementados
#include "Altimeter.h"
#include "SpeedIndicator.h"

// TODO: Agregar includes de futuros instrumentos
// #include "AttitudeIndicator.h"
// #include "AirspeedIndicator.h"
// #include "HeadingIndicator.h"
// #include "VerticalSpeedIndicator.h"

namespace hud
{
    /**
     * @class FlightHUD
     * @brief Coordinador central de todos los instrumentos del HUD
     *
     * Esta clase es el punto de entrada para el sistema de HUD. Gestiona:
     * - Creación y configuración de todos los instrumentos
     * - Actualización de datos de vuelo
     * - Renderizado coordinado de todos los instrumentos
     * - Layout y posicionamiento
     * - Manejo de cambios de resolución
     */
    class FlightHUD
    {
    public:
        FlightHUD();
        ~FlightHUD() = default;

        // ========================================================================
        // INICIALIZACIÓN Y CONFIGURACIÓN
        // ========================================================================

        void init(int screenWidth, int screenHeight);
        void setScreenSize(int width, int height);
        void setLayout(const std::string &layoutName);

        // ========================================================================
        // ACTUALIZACIÓN Y RENDERIZADO
        // ========================================================================

        void update(const flight::FlightData &flightData);
        void render();

    private:
        // ========================================================================
        // SISTEMA DE RENDERIZADO
        // ========================================================================

        std::unique_ptr<gfx::Renderer2D> renderer2D_;

        // ========================================================================
        // INSTRUMENTOS DEL HUD
        // ========================================================================

        // Contenedor polimórfico de instrumentos
        // Permite gestionar todos los instrumentos de forma uniforme
        std::vector<std::unique_ptr<Instrument>> instruments_;

        // Referencias rápidas a instrumentos específicos (opcional)
        // Útil para configuración directa sin recorrer el vector
        Altimeter* altimeter_;
        SpeedIndicator* speedIndicator_;

        // TODO: Agregar referencias a futuros instrumentos aquí
        // AttitudeIndicator* attitudeIndicator_;
        // HeadingIndicator* headingIndicator_;
        // VerticalSpeedIndicator* verticalSpeedIndicator_;

        // ========================================================================
        // DATOS Y CONFIGURACIÓN
        // ========================================================================

        flight::FlightData currentFlightData_;
        int screenWidth_;
        int screenHeight_;

        // ========================================================================
        // ESQUEMA DE COLORES DEL HUD
        // ========================================================================

        glm::vec4 hudColor_;     // Color principal (verde HUD)
        glm::vec4 warningColor_; // Color de advertencia (amarillo/ámbar)
        glm::vec4 dangerColor_;  // Color de peligro (rojo)

        // ========================================================================
        // CONFIGURACIÓN INTERNA
        // ========================================================================

        void setupInstrumentLayout(); // Configura layout de TODOS los instrumentos
    };

} // namespace hud
