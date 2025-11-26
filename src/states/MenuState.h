/**
 * @file MenuState.h
 * @brief Main menu state for mission selection.
 */

#pragma once

#include "states/IModeState.h"

namespace states
{

    /**
     * @class MenuState
     * @brief Handles the main menu of the simulator.
     *
     * Displays the list of available missions and allows the user to select one.
     */
    class MenuState : public IModeState
    {
    public:
        void onEnter(core::AppContext &context) override;
        void handleInput(core::AppContext &context) override;
        void update(core::AppContext &context) override;
        void render(core::AppContext &context) override;

    private:
        bool menuStateRestored_ = false;
    };

} // namespace states
