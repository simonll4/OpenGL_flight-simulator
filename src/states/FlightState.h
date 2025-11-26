/**
 * @file FlightState.h
 * @brief Active flight simulation state.
 */

#pragma once

#include "states/IModeState.h"

namespace states
{

    /**
     * @class FlightState
     * @brief Handles the active flight simulation mode.
     *
     * Manages the simulation loop, including input handling, physics updates,
     * rendering of the 3D world and HUD, and mission progress tracking.
     */
    class FlightState : public IModeState
    {
    public:
        void onEnter(core::AppContext &context) override;
        void handleInput(core::AppContext &context) override;
        void update(core::AppContext &context) override;
        void render(core::AppContext &context) override;

    private:
        void restartMission(core::AppContext &context);

        bool escPressed_ = false;
        bool tabPressed_ = false;
        bool mPressed_ = false;
        bool rPressed_ = false;
        bool overlayEscPressed_ = false;
        bool completionPromptShown_ = false;
    };

} // namespace states
