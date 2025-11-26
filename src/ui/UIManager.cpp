#include "ui/UIManager.h"

#include <stdexcept>

namespace ui
{

    UIManager::UIManager() = default;
    UIManager::~UIManager() = default;

    bool UIManager::initialize(int width, int height, mission::MissionRegistry *registry)
    {
        registry_ = registry;
        screenWidth_ = width;
        screenHeight_ = height;

        // Main HUD: create default layout and initialize with current resolution
        hud_ = std::make_unique<hud::FlightHUD>();
        hud_->init(width, height);
        hud_->setLayout("classic");

        // Menu feeds from mission registry to list available scenarios
        menu_ = std::make_unique<ui::MissionMenu>();
        menu_->init(registry_, width, height);

        // Planner reuses menu renderer to share resources (fonts, textures)
        planner_ = std::make_unique<ui::MissionPlanner>();
        planner_->init(width, height, menu_->getRenderer());

        // Overlay handles briefing/completion over 3D view
        overlay_ = std::make_unique<ui::MissionOverlay>();
        overlay_->init(width, height);

        return true;
    }

    void UIManager::resize(int width, int height)
    {
        // Save current resolution for future layout calculations
        screenWidth_ = width;
        screenHeight_ = height;
        if (hud_)
        {
            // FlightHUD recalculates grids and relative positions
            hud_->setScreenSize(width, height);
        }
        if (menu_)
        {
            // Menus depend on resolution for navigation mesh
            menu_->setScreenSize(width, height);
        }
        if (planner_)
        {
            planner_->setScreenSize(width, height);
        }
        if (overlay_)
        {
            overlay_->setScreenSize(width, height);
        }
    }

    void UIManager::updateMenu(GLFWwindow *window, float dt)
    {
        // Forward input and time delta to selection menu
        if (menu_)
        {
            menu_->update(window, dt);
        }
    }

    void UIManager::renderMenu()
    {
        // Menu uses its own Renderer2D, just need to trigger draw call
        if (menu_)
        {
            menu_->render();
        }
    }

    ui::MenuResult UIManager::getMenuResult() const
    {
        // Returns result of last frame (start, exit, etc.)
        if (menu_)
        {
            return menu_->getResult();
        }
        return ui::MenuResult{};
    }

    void UIManager::resetMenu()
    {
        // Clears selections and transient menu states
        if (menu_)
        {
            menu_->reset();
        }
    }

    void UIManager::preselectMission(int index)
    {
        // Allows marking a mission when coming from planner or other flow
        if (menu_)
        {
            menu_->preselectMission(index);
        }
    }

    void UIManager::updatePlanner(GLFWwindow *window, float dt)
    {
        // Planner responds to input for waypoint manipulation
        if (planner_)
        {
            planner_->update(window, dt);
        }
    }

    void UIManager::renderPlanner()
    {
        // Draws map, waypoints, and additional UI
        if (planner_)
        {
            planner_->render();
        }
    }

    ui::PlannerResult UIManager::getPlannerResult() const
    {
        // Reports user action (accept plan, return, etc.)
        if (planner_)
        {
            return planner_->getResult();
        }
        return ui::PlannerResult{};
    }

    void UIManager::resetPlanner()
    {
        // Clears route and temporary states when leaving planner
        if (planner_)
        {
            planner_->reset();
        }
    }

    void UIManager::loadPlannerMission(const mission::MissionDefinition &mission)
    {
        // Preloads a mission for editing within planner
        if (planner_)
        {
            planner_->loadMission(mission);
        }
    }

    mission::MissionDefinition UIManager::getPlannerMission() const
    {
        // Returns currently edited mission (copy)
        if (planner_)
        {
            return planner_->getMission();
        }
        return mission::MissionDefinition();
    }

    void UIManager::updateOverlay(float dt)
    {
        // Overlay only needs time delta for animations
        if (overlay_)
        {
            overlay_->update(dt);
        }
    }

    bool UIManager::handleOverlayInput(GLFWwindow *window)
    {
        // Handles clicks/keys when overlay is visible
        if (overlay_)
        {
            return overlay_->handleInput(window);
        }
        return false;
    }

    bool UIManager::isOverlayVisible() const
    {
        return overlay_ && overlay_->isVisible();
    }

    bool UIManager::overlayReadyToFly() const
    {
        return overlay_ && overlay_->readyToFly();
    }

    ui::CompletionChoice UIManager::overlayChoice() const
    {
        if (overlay_)
        {
            return overlay_->getCompletionChoice();
        }
        return ui::CompletionChoice::None;
    }

    void UIManager::showBriefing(const mission::MissionDefinition &mission)
    {
        // Shows pre-flight briefing with selected mission data
        if (overlay_)
        {
            overlay_->showBriefing(mission);
        }
    }

    void UIManager::showCompletionPrompt(const mission::MissionRuntime &runtime)
    {
        // Presents current flight results and continuation options
        if (overlay_)
        {
            overlay_->showCompletionPrompt(runtime);
        }
    }

    void UIManager::hideOverlay()
    {
        // Hides overlay without destroying it (allows quick re-show)
        if (overlay_)
        {
            overlay_->hide();
        }
    }

    void UIManager::resetOverlay()
    {
        // Clears internal state (previous selections, texts, etc.)
        if (overlay_)
        {
            overlay_->reset();
        }
    }

    void UIManager::renderOverlay()
    {
        // Draws semi-transparent panels over 3D scene
        if (overlay_)
        {
            overlay_->render();
        }
    }

    void UIManager::updateHUD(const flight::FlightData &data)
    {
        // Propagates most recent flight data to HUD
        if (hud_)
        {
            hud_->update(data);
        }
    }

    void UIManager::renderHUD()
    {
        // Renders HUD on top of other elements
        if (hud_)
        {
            hud_->render();
        }
    }

} // namespace ui
