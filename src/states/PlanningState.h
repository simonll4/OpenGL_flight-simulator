/**
 * @file PlanningState.h
 * @brief Mission planning and briefing state.
 */

#pragma once

#include "states/IModeState.h"

namespace states
{

    /**
     * @class PlanningState
     * @brief Handles the mission planning phase.
     *
     * Shows the mission briefing and map, allowing the user to review
     * waypoints before starting the flight.
     */
    class PlanningState : public IModeState
    {
    public:
        void onEnter(core::AppContext &context) override;
        void handleInput(core::AppContext &context) override;
        void update(core::AppContext &context) override;
        void render(core::AppContext &context) override;
    };

} // namespace states
