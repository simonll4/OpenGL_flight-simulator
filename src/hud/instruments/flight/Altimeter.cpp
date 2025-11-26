#include "Altimeter.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <cmath>

namespace hud
{
    ////////////////////////////////////////////////////////////////////////////
    //  Tape Scale Configuration
    ////////////////////////////////////////////////////////////////////////////

    // Spacing between altitude marks
    static const float ALTITUDE_STEP = 100.0f;  // Marks every 100 feet
    static const float PIXELS_PER_STEP = 30.0f; // Vertical separation between marks
    static const int VISIBLE_MARKS = 12;        // How many marks to show above/below center

    ////////////////////////////////////////////////////////////////////////////
    //  Tape Visual Configuration
    ////////////////////////////////////////////////////////////////////////////

    // Ticks (lateral marks)
    static const float TICK_LENGTH = 16.0f;       // Length of horizontal marks
    static const float TICK_TO_NUMBER_GAP = 6.0f; // Gap between tick and number

    // Central readout box
    static const float READOUT_BOX_WIDTH = 120.0f;
    static const float READOUT_BOX_HEIGHT = 44.0f;

    // Indicator arrow (chevron)
    static const float CHEVRON_WIDTH = 10.0f;
    static const float CHEVRON_HEIGHT = 12.0f;

    Altimeter::Altimeter() : Instrument()
    {
        // Altimeter-specific configuration
        size_ = glm::vec2(100.0f, 400.0f);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // HUD Green
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Main Render Function
    ////////////////////////////////////////////////////////////////////////////

    void Altimeter::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        if (!enabled_)
            return;

        float altitude = flightData.altitude;

        drawBackground(renderer);
        drawAltitudeTape(renderer, altitude);
        drawCurrentAltitudeBox(renderer, altitude);
    }

    void Altimeter::drawBackground(gfx::Renderer2D &renderer)
    {
        // Altimeter has no background - only draw elements on transparent HUD
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Altitude Tape (Moving Scale)
    ////////////////////////////////////////////////////////////////////////////

    void Altimeter::drawAltitudeTape(gfx::Renderer2D &renderer, float altitude)
    {
        // Calculate reference positions
        float centerY = position_.y + size_.y * 0.5f; // Vertical center of instrument
        float ticksX = position_.x + size_.x - 15.0f; // Column where ticks go

        // Calculate tape offset based on current altitude
        float baseAltitude = floor(altitude / ALTITUDE_STEP) * ALTITUDE_STEP;
        float fraction = (altitude - baseAltitude) / ALTITUDE_STEP;
        float scrollOffset = fraction * PIXELS_PER_STEP;

        // Draw visible altitude marks
        for (int i = -VISIBLE_MARKS; i <= VISIBLE_MARKS; ++i)
        {
            int markAltitude = (int)baseAltitude + i * (int)ALTITUDE_STEP;
            float markY = centerY + scrollOffset - i * PIXELS_PER_STEP;

            const float CULLING_MARGIN = 30.0f;
            if (markY < position_.y - CULLING_MARGIN || markY > position_.y + size_.y + CULLING_MARGIN)
                continue;

            bool insideReadoutBox = (markY > centerY - READOUT_BOX_HEIGHT * 0.5f &&
                                     markY < centerY + READOUT_BOX_HEIGHT * 0.5f);

            if (insideReadoutBox)
                continue;

            renderer.drawRect(
                glm::vec2(ticksX - TICK_LENGTH, markY - 0.5f),
                glm::vec2(TICK_LENGTH, 1.0f),
                color_,
                true);

            if (markAltitude >= 0)
            {
                float numberX = ticksX - TICK_LENGTH - TICK_TO_NUMBER_GAP - 30.0f;
                glm::vec2 numberPos = glm::vec2(numberX, markY);
                gfx::TextRenderer::drawString(renderer, std::to_string(markAltitude), numberPos, glm::vec2(8.0f, 12.0f), color_, 10.0f);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Digital Readout Box
    ////////////////////////////////////////////////////////////////////////////

    void Altimeter::drawCurrentAltitudeBox(gfx::Renderer2D &renderer, float altitude)
    {
        float centerY = position_.y + size_.y * 0.5f;

        float boxX = position_.x + (size_.x - READOUT_BOX_WIDTH) * 0.5f;
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_,
            false);

        float chevronX = boxX - CHEVRON_WIDTH;
        float chevronTopY = centerY - CHEVRON_HEIGHT * 0.5f;
        float chevronBotY = centerY + CHEVRON_HEIGHT * 0.5f;

        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(boxX, centerY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(boxX, centerY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);

        int displayAltitude = (int)round(altitude);
        if (displayAltitude < 0)
            displayAltitude = 0;

        glm::vec2 numberPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        gfx::TextRenderer::drawString(renderer, std::to_string(displayAltitude), numberPos, glm::vec2(8.0f, 12.0f), color_, 10.0f);
    }

} // namespace hud
