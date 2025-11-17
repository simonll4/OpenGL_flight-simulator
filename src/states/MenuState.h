#pragma once

#include "states/IModeState.h"

namespace states
{

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
