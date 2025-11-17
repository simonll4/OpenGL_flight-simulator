#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../../gfx/rendering/Renderer2D.h"
#include "../../flight/data/FlightData.h"
#include "Instrument.h"

// Includes de instrumentos implementados
#include "../instruments/flight/Altimeter.h"
#include "../instruments/flight/SpeedIndicator.h"
#include "../instruments/flight/VerticalSpeedIndicator.h"
#include "../instruments/navigation/WaypointIndicator.h"
#include "../instruments/attitude/BankAngleIndicator.h"
#include "../instruments/attitude/PitchLadder.h"

// TODO: Agregar includes de futuros instrumentos
// #include "HeadingIndicator.h"

namespace hud
{
    /**
     * @class FlightHUD
     * @brief Coordinador central de todos los instrumentos del HUD.
     *
     * Se encarga de instanciar cada instrumento, compartir un `Renderer2D`
     * común, aplicar layouts responsivos y despachar datos de vuelo antes del
     * render. Actúa como fachada entre la simulación y la capa de UI.
     */
    class FlightHUD
    {
    public:
        FlightHUD();
        ~FlightHUD() = default;

        // ========================================================================
        // INICIALIZACIÓN Y CONFIGURACIÓN
        // ========================================================================
        /**
         * @brief Inicializa el renderer 2D y aplica layout inicial.
         * @param screenWidth Ancho del framebuffer en píxeles.
         * @param screenHeight Alto del framebuffer en píxeles.
         */
        void init(int screenWidth, int screenHeight);

        /// Ajusta matrices y layouts cuando cambia la resolución del HUD.
        void setScreenSize(int width, int height);

        /// Permite alternar layouts predefinidos (classic/modern/minimal).
        void setLayout(const std::string &layoutName);

        // ========================================================================
        // ACTUALIZACIÓN Y RENDERIZADO
        // ========================================================================

        /// Copia los datos de vuelo más recientes para consumo de los gauges.
        void update(const flight::FlightData &flightData);

        /// Renderiza todos los instrumentos como overlay 2D.
        void render();

    private:
        // ========================================================================
        // SISTEMA DE RENDERIZADO
        // ========================================================================

        std::unique_ptr<gfx::Renderer2D> renderer2D_; ///< Renderer compartido entre instrumentos.

        // ========================================================================
        // INSTRUMENTOS DEL HUD
        // ========================================================================

        // Contenedor polimórfico de instrumentos
        // Permite gestionar todos los instrumentos de forma uniforme
        std::vector<std::unique_ptr<Instrument>> instruments_; ///< Pool polimórfico (propiedad).

        // Referencias rápidas a instrumentos específicos (opcional)
        // Útil para configuración directa sin recorrer el vector
        Altimeter *altimeter_;
        SpeedIndicator *speedIndicator_;
        VerticalSpeedIndicator *verticalSpeedIndicator_;
        WaypointIndicator *waypointIndicator_;
        BankAngleIndicator *bankAngleIndicator_;
        PitchLadder *pitchLadder_;

        // TODO: Agregar referencias a futuros instrumentos aquí
        // HeadingIndicator* headingIndicator_;

        // ========================================================================
        // DATOS Y CONFIGURACIÓN
        // ========================================================================

        flight::FlightData currentFlightData_; ///< Copia local para sincronizar render.
        int screenWidth_;
        int screenHeight_;

        // ========================================================================
        // ESQUEMA DE COLORES DEL HUD
        // ========================================================================

        glm::vec4 hudColor_;     ///< Color principal (verde HUD).
        glm::vec4 warningColor_; ///< Color de advertencia (amarillo/ámbar).
        glm::vec4 dangerColor_;  ///< Color de peligro (rojo).

        // ========================================================================
        // CONFIGURACIÓN INTERNA
        // ========================================================================

        /// Calcula posiciones/tamaños para todos los instrumentos actuales.
        void setupInstrumentLayout();
    };

} // namespace hud
