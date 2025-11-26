/**
 * @file FlightHUD.h
 * @brief Central coordinator for all HUD instruments.
 */

#pragma once
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../../gfx/rendering/Renderer2D.h"
#include "../../flight/data/FlightData.h"
#include "Instrument.h"

// Includes of implemented instruments
#include "../instruments/flight/Altimeter.h"
#include "../instruments/flight/SpeedIndicator.h"
#include "../instruments/flight/VerticalSpeedIndicator.h"
#include "../instruments/navigation/WaypointIndicator.h"
#include "../instruments/attitude/BankAngleIndicator.h"
#include "../instruments/attitude/PitchLadder.h"

// TODO: Add includes for future instruments
// #include "HeadingIndicator.h"

namespace hud
{
    /**
     * @class FlightHUD
     * @brief Central coordinator for all HUD instruments.
     *
     * Responsible for instantiating each instrument, sharing a common `Renderer2D`,
     * applying responsive layouts, and dispatching flight data before rendering.
     * Acts as a facade between the simulation and the UI layer.
     */
    class FlightHUD
    {
    public:
        FlightHUD();
        ~FlightHUD() = default;

        // ========================================================================
        // INITIALIZATION AND CONFIGURATION
        // ========================================================================
        /**
         * @brief Initializes the 2D renderer and applies initial layout.
         * @param screenWidth Framebuffer width in pixels.
         * @param screenHeight Framebuffer height in pixels.
         */
        void init(int screenWidth, int screenHeight);

        /// Adjusts matrices and layouts when HUD resolution changes.
        void setScreenSize(int width, int height);

        /// Allows toggling predefined layouts (classic/modern/minimal).
        void setLayout(const std::string &layoutName);

        // ========================================================================
        // UPDATE AND RENDERING
        // ========================================================================

        /// Copies the most recent flight data for gauge consumption.
        void update(const flight::FlightData &flightData);

        /// Renders all instruments as a 2D overlay.
        void render();

    private:
        // ========================================================================
        // RENDERING SYSTEM
        // ========================================================================

        std::unique_ptr<gfx::Renderer2D> renderer2D_; ///< Renderer shared among instruments.

        // ========================================================================
        // HUD INSTRUMENTS
        // ========================================================================

        // Polymorphic container of instruments
        // Allows managing all instruments uniformly
        std::vector<std::unique_ptr<Instrument>> instruments_; ///< Polymorphic pool (ownership).

        // Quick references to specific instruments (optional)
        // Useful for direct configuration without traversing the vector
        Altimeter *altimeter_;
        SpeedIndicator *speedIndicator_;
        VerticalSpeedIndicator *verticalSpeedIndicator_;
        WaypointIndicator *waypointIndicator_;
        BankAngleIndicator *bankAngleIndicator_;
        PitchLadder *pitchLadder_;

        // TODO: Add references to future instruments here
        // HeadingIndicator* headingIndicator_;

        // ========================================================================
        // DATA AND CONFIGURATION
        // ========================================================================

        flight::FlightData currentFlightData_; ///< Local copy to synchronize render.
        int screenWidth_;
        int screenHeight_;

        // ========================================================================
        // HUD COLOR SCHEME
        // ========================================================================

        glm::vec4 hudColor_;     ///< Main color (HUD green).
        glm::vec4 warningColor_; ///< Warning color (yellow/amber).
        glm::vec4 dangerColor_;  ///< Danger color (red).

        // ========================================================================
        // INTERNAL CONFIGURATION
        // ========================================================================

        /// Calculates positions/sizes for all current instruments.
        void setupInstrumentLayout();
    };

} // namespace hud
