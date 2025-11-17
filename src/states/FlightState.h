#pragma once

#include "states/IModeState.h"

namespace states
{

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
