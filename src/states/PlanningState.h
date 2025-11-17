#pragma once

#include "states/IModeState.h"

namespace states {

class PlanningState : public IModeState {
public:
    void onEnter(core::AppContext& context) override;
    void handleInput(core::AppContext& context) override;
    void update(core::AppContext& context) override;
    void render(core::AppContext& context) override;
};

} // namespace states
