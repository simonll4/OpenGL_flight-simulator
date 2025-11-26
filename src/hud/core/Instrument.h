/**
 * @file Instrument.h
 * @brief Abstract base class for HUD instruments.
 */

#pragma once
#include <glm/glm.hpp>
#include "../../gfx/rendering/Renderer2D.h"
#include "../../flight/data/FlightData.h"

namespace hud
{
    /**
     * @class Instrument
     * @brief Abstract base class for all HUD instruments.
     *
     * This class defines the common interface for all flight instruments.
     * It provides:
     * - Common properties (position, size, color) expressed in pixels relative to the HUD viewport.
     * - Configuration methods (setters) to integrate with layouts calculated by FlightHUD.
     * - Rendering interface (pure virtual method) that receives the Renderer2D and updated flight data.
     *
     * Each specific instrument (Altimeter, AttitudeIndicator, etc.)
     * must inherit from this class and implement its own render() method.
     */
    class Instrument
    {
    public:
        /**
         * @brief Default constructor.
         * Initializes the instrument with default values.
         */
        Instrument();

        /**
         * @brief Virtual destructor to allow polymorphism.
         */
        virtual ~Instrument() = default;

        // ====================================================================
        // COMMON CONFIGURATION METHODS
        // ====================================================================

        /**
         * @brief Sets the instrument position in screen coordinates.
         * @param position Position (x, y) of the top-left corner.
         *
         * The position equates to the local origin used by each instrument to convert
         * Normalized Device Coordinates (NDC) to pixels.
         */
        void setPosition(const glm::vec2 &position) { position_ = position; }

        /**
         * @brief Sets the instrument size.
         * @param size Dimensions (width, height) of the instrument.
         *
         * Instruments use this size to scale geometry proportionally
         * and maintain a consistent appearance at any resolution.
         */
        void setSize(const glm::vec2 &size) { size_ = size; }

        /**
         * @brief Sets the main color of the instrument.
         * @param color RGBA color (values between 0.0 and 1.0).
         *
         * Typically a translucent green is used to mimic real HUDs, but the
         * property allows variations for special states or alternate themes.
         */
        void setColor(const glm::vec4 &color) { color_ = color; }

        /**
         * @brief Enables or disables the instrument display.
         * @param enabled true to show, false to hide.
         *
         * FlightHUD uses this flag to not invoke render() when a module is
         * configured as optional or temporarily unavailable.
         */
        void setEnabled(bool enabled) { enabled_ = enabled; }

        // ====================================================================
        // ACCESS METHODS (GETTERS)
        // ====================================================================

        const glm::vec2 &getPosition() const { return position_; }
        const glm::vec2 &getSize() const { return size_; }
        const glm::vec4 &getColor() const { return color_; }
        bool isEnabled() const { return enabled_; }

        // ====================================================================
        // RENDERING INTERFACE
        // ====================================================================

        /**
         * @brief Renders the instrument on screen.
         * @param renderer Shared 2D renderer.
         * @param flightData Current flight data.
         *
         * This method must be implemented by each specific instrument.
         * It is responsible for drawing all visual elements of the instrument.
         */
        virtual void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) = 0;

    protected:
        // ====================================================================
        // COMMON PROPERTIES FOR ALL INSTRUMENTS
        // ====================================================================

        glm::vec2 position_; ///< Screen position (x, y).
        glm::vec2 size_;     ///< Instrument size (width, height).
        glm::vec4 color_;    ///< Main RGBA color.
        bool enabled_;       ///< Whether the instrument is active/visible.
    };

} // namespace hud
