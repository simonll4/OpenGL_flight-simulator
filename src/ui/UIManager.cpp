#include "ui/UIManager.h"

#include <stdexcept>

namespace ui {

UIManager::UIManager() = default;
UIManager::~UIManager() = default;

bool UIManager::initialize(int width, int height, mission::MissionRegistry* registry) {
    registry_ = registry;
    screenWidth_ = width;
    screenHeight_ = height;

    hud_ = std::make_unique<hud::FlightHUD>();
    hud_->init(width, height);
    hud_->setLayout("classic");

    menu_ = std::make_unique<ui::MissionMenu>();
    menu_->init(registry_, width, height);

    planner_ = std::make_unique<ui::MissionPlanner>();
    planner_->init(width, height, menu_->getRenderer());

    overlay_ = std::make_unique<ui::MissionOverlay>();
    overlay_->init(width, height);

    return true;
}

void UIManager::resize(int width, int height) {
    screenWidth_ = width;
    screenHeight_ = height;
    if (hud_) {
        hud_->setScreenSize(width, height);
    }
    if (menu_) {
        menu_->setScreenSize(width, height);
    }
    if (planner_) {
        planner_->setScreenSize(width, height);
    }
    if (overlay_) {
        overlay_->setScreenSize(width, height);
    }
}

void UIManager::updateMenu(GLFWwindow* window, float dt) {
    if (menu_) {
        menu_->update(window, dt);
    }
}

void UIManager::renderMenu() {
    if (menu_) {
        menu_->render();
    }
}

ui::MenuResult UIManager::getMenuResult() const {
    if (menu_) {
        return menu_->getResult();
    }
    return ui::MenuResult{};
}

void UIManager::resetMenu() {
    if (menu_) {
        menu_->reset();
    }
}

void UIManager::preselectMission(int index) {
    if (menu_) {
        menu_->preselectMission(index);
    }
}

void UIManager::updatePlanner(GLFWwindow* window, float dt) {
    if (planner_) {
        planner_->update(window, dt);
    }
}

void UIManager::renderPlanner() {
    if (planner_) {
        planner_->render();
    }
}

ui::PlannerResult UIManager::getPlannerResult() const {
    if (planner_) {
        return planner_->getResult();
    }
    return ui::PlannerResult{};
}

void UIManager::resetPlanner() {
    if (planner_) {
        planner_->reset();
    }
}

void UIManager::loadPlannerMission(const mission::MissionDefinition& mission) {
    if (planner_) {
        planner_->loadMission(mission);
    }
}

mission::MissionDefinition UIManager::getPlannerMission() const {
    if (planner_) {
        return planner_->getMission();
    }
    return mission::MissionDefinition();
}

void UIManager::updateOverlay(float dt) {
    if (overlay_) {
        overlay_->update(dt);
    }
}

bool UIManager::handleOverlayInput(GLFWwindow* window) {
    if (overlay_) {
        return overlay_->handleInput(window);
    }
    return false;
}

bool UIManager::isOverlayVisible() const {
    return overlay_ && overlay_->isVisible();
}

bool UIManager::overlayReadyToFly() const {
    return overlay_ && overlay_->readyToFly();
}

ui::CompletionChoice UIManager::overlayChoice() const {
    if (overlay_) {
        return overlay_->getCompletionChoice();
    }
    return ui::CompletionChoice::None;
}

void UIManager::showBriefing(const mission::MissionDefinition& mission) {
    if (overlay_) {
        overlay_->showBriefing(mission);
    }
}

void UIManager::showCompletionPrompt(const mission::MissionRuntime& runtime) {
    if (overlay_) {
        overlay_->showCompletionPrompt(runtime);
    }
}

void UIManager::hideOverlay() {
    if (overlay_) {
        overlay_->hide();
    }
}

void UIManager::resetOverlay() {
    if (overlay_) {
        overlay_->reset();
    }
}

void UIManager::renderOverlay() {
    if (overlay_) {
        overlay_->render();
    }
}

void UIManager::updateHUD(const flight::FlightData& data) {
    if (hud_) {
        hud_->update(data);
    }
}

void UIManager::renderHUD() {
    if (hud_) {
        hud_->render();
    }
}

} // namespace ui
