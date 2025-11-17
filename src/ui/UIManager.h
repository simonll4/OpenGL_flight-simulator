#pragma once

#include <memory>

#include "hud/core/FlightHUD.h"
#include "ui/menu/MissionMenu.h"
#include "ui/menu/MissionPlanner.h"
#include "ui/overlay/MissionOverlay.h"
#include "flight/data/FlightData.h"

struct GLFWwindow;

namespace mission {
class MissionRegistry;
class MissionDefinition;
class MissionRuntime;
}

namespace ui {

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool initialize(int width, int height, mission::MissionRegistry* registry);
    void resize(int width, int height);

    void updateMenu(GLFWwindow* window, float dt);
    void renderMenu();
    ui::MenuResult getMenuResult() const;
    void resetMenu();
    void preselectMission(int index);

    void updatePlanner(GLFWwindow* window, float dt);
    void renderPlanner();
    ui::PlannerResult getPlannerResult() const;
    void resetPlanner();
    void loadPlannerMission(const mission::MissionDefinition& mission);
    mission::MissionDefinition getPlannerMission() const;

    void updateOverlay(float dt);
    bool handleOverlayInput(GLFWwindow* window);
    bool isOverlayVisible() const;
    bool overlayReadyToFly() const;
    ui::CompletionChoice overlayChoice() const;
    void showBriefing(const mission::MissionDefinition& mission);
    void showCompletionPrompt(const mission::MissionRuntime& runtime);
    void hideOverlay();
    void resetOverlay();
    void renderOverlay();

    hud::FlightHUD& hud() { return *hud_; }
    void updateHUD(const flight::FlightData& data);
    void renderHUD();

private:
    std::unique_ptr<hud::FlightHUD> hud_;
    std::unique_ptr<ui::MissionMenu> menu_;
    std::unique_ptr<ui::MissionPlanner> planner_;
    std::unique_ptr<ui::MissionOverlay> overlay_;

    mission::MissionRegistry* registry_ = nullptr;
    int screenWidth_ = 1280;
    int screenHeight_ = 720;
};

} // namespace ui
