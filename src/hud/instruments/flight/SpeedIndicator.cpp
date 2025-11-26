#include "SpeedIndicator.h"
#include "../../../gfx/rendering/TextRenderer.h"
#include <cmath>

namespace hud
{
    ////////////////////////////////////////////////////////////////////////////
    //  Tape Scale Configuration
    ////////////////////////////////////////////////////////////////////////////

    static const float SPEED_STEP = 10.0f;      // Marks every 10 knots
    static const float PIXELS_PER_STEP = 30.0f; // Vertical separation between marks
    static const int VISIBLE_MARKS = 12;        // How many marks to show above/below

    // Visual configuration
    static const float TICK_LENGTH = 16.0f;
    static const float TICK_TO_NUMBER_GAP = 6.0f;
    static const float READOUT_BOX_WIDTH = 100.0f;
    static const float READOUT_BOX_HEIGHT = 44.0f;
    static const float CHEVRON_WIDTH = 10.0f;
    static const float CHEVRON_HEIGHT = 12.0f;

    SpeedIndicator::SpeedIndicator() : Instrument()
    {
        // Speed indicator specific configuration
        size_ = glm::vec2(100.0f, 400.0f);
        color_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // HUD Green
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Main Render
    ////////////////////////////////////////////////////////////////////////////

    void SpeedIndicator::render(gfx::Renderer2D &renderer, const flight::FlightData &flightData)
    {
        // Only render if enabled
        if (!enabled_)
            return;

        float airspeed = flightData.airspeed;

        drawSpeedTape(renderer, airspeed);
        drawCurrentSpeedBox(renderer, airspeed);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Speed Tape (Moving Scale)
    ////////////////////////////////////////////////////////////////////////////

    void SpeedIndicator::drawSpeedTape(gfx::Renderer2D &renderer, float airspeed)
    {
        // Calculate instrument vertical center (inherited from Instrument)
        float centerY = position_.y + size_.y * 0.5f;
        float ticksX = position_.x + 15.0f; // Ticks column on the left

        // Calculate tape offset
        float baseSpeed = floor(airspeed / SPEED_STEP) * SPEED_STEP;
        float fraction = (airspeed - baseSpeed) / SPEED_STEP;
        float scrollOffset = fraction * PIXELS_PER_STEP;

        // Draw visible speed marks
        for (int i = -VISIBLE_MARKS; i <= VISIBLE_MARKS; ++i)
        {
            int markSpeed = (int)baseSpeed + i * (int)SPEED_STEP;

            // Skip negative speeds
            if (markSpeed < 0)
                continue;

            float markY = centerY + scrollOffset - i * PIXELS_PER_STEP;

            // Cull marks outside visible area
            const float CULLING_MARGIN = 30.0f;
            if (markY < position_.y - CULLING_MARGIN || markY > position_.y + size_.y + CULLING_MARGIN)
                continue;

            // Skip marks inside readout box
            bool insideReadoutBox = (markY > centerY - READOUT_BOX_HEIGHT * 0.5f &&
                                     markY < centerY + READOUT_BOX_HEIGHT * 0.5f);
            if (insideReadoutBox)
                continue;

            // Draw tick
            renderer.drawRect(
                glm::vec2(ticksX, markY - 0.5f),
                glm::vec2(TICK_LENGTH, 1.0f),
                color_,
                true);

            // Draw number (only every 20 knots to avoid clutter)
            if (markSpeed % 20 == 0)
            {
                float numberX = ticksX + TICK_LENGTH + TICK_TO_NUMBER_GAP + 20.0f;
                glm::vec2 numberPos = glm::vec2(numberX, markY);
                drawSpeedNumber(renderer, markSpeed, numberPos);
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Digital Readout Box
    ////////////////////////////////////////////////////////////////////////////

    void SpeedIndicator::drawCurrentSpeedBox(gfx::Renderer2D &renderer, float airspeed)
    {
        float centerY = position_.y + size_.y * 0.5f;

        // Position box in center
        float boxX = position_.x + (size_.x - READOUT_BOX_WIDTH) * 0.5f;
        float boxY = centerY - READOUT_BOX_HEIGHT * 0.5f;

        // Draw box frame
        renderer.drawRect(
            glm::vec2(boxX, boxY),
            glm::vec2(READOUT_BOX_WIDTH, READOUT_BOX_HEIGHT),
            color_,
            false // Border only
        );

        // Draw chevron to the right of the box
        float chevronX = boxX + READOUT_BOX_WIDTH;
        float chevronTopY = centerY - CHEVRON_HEIGHT * 0.5f;
        float chevronBotY = centerY + CHEVRON_HEIGHT * 0.5f;

        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX + CHEVRON_WIDTH, centerY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX + CHEVRON_WIDTH, centerY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);
        renderer.drawLine(
            glm::vec2(chevronX, chevronTopY),
            glm::vec2(chevronX, chevronBotY),
            color_, 2.0f);

        // Show current speed rounded
        int displaySpeed = (int)round(airspeed);
        if (displaySpeed < 0)
            displaySpeed = 0;

        // Draw centered number
        glm::vec2 numberPos = glm::vec2(boxX + READOUT_BOX_WIDTH * 0.5f, centerY);
        drawSpeedNumber(renderer, displaySpeed, numberPos);
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Number Rendering (Helper)
    ////////////////////////////////////////////////////////////////////////////

    void SpeedIndicator::drawSpeedNumber(gfx::Renderer2D &renderer, int speed, const glm::vec2 &position)
    {
        static const glm::vec2 DIGIT_SIZE = glm::vec2(8.0f, 12.0f);
        static const float DIGIT_SPACING = 10.0f;

        gfx::TextRenderer::drawString(
            renderer,
            std::to_string(speed),
            position,
            DIGIT_SIZE,
            color_,
            DIGIT_SPACING);
    }

} // namespace hud
