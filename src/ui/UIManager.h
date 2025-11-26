/**
 * @file UIManager.h
 * @brief UI orchestration for all simulator panels and overlays.
 */

#pragma once

#include <memory>

#include "hud/core/FlightHUD.h"
#include "ui/menu/MissionMenu.h"
#include "ui/menu/MissionPlanner.h"
#include "ui/overlay/MissionOverlay.h"
#include "flight/data/FlightData.h"

struct GLFWwindow;

namespace mission
{
    class MissionRegistry;
    class MissionDefinition;
    class MissionRuntime;
}

namespace ui
{

    /**
     * @class UIManager
     * @brief Orchestration point for all simulator UI panels.
     *
     * Encapsulates creation, lifecycle, and communication between:
     * - Main HUD (FlightHUD)
     * - Mission selection menu
     * - Route planner (MissionPlanner)
     * - In-game overlay (briefing/completion)
     *
     * Exposes separate update/render methods per subsystem so each game state
     * invokes only what it needs.
     */
    class UIManager
    {
    public:
        UIManager();
        ~UIManager();

        /**
         * @brief Initializes HUD and panels with initial dimensions and access to mission registry.
         */
        bool initialize(int width, int height, mission::MissionRegistry *registry);

        /**
         * @brief Recalculates layouts when main window size changes.
         */
        void resize(int width, int height);

        // ============================ MAIN MENU ============================
        void updateMenu(GLFWwindow *window, float dt);
        void renderMenu();
        ui::MenuResult getMenuResult() const;
        void resetMenu();
        void preselectMission(int index);

        // ============================ PLANNER ==============================
        void updatePlanner(GLFWwindow *window, float dt);
        void renderPlanner();
        ui::PlannerResult getPlannerResult() const;
        void resetPlanner();
        void loadPlannerMission(const mission::MissionDefinition &mission);
        mission::MissionDefinition getPlannerMission() const;

        // ============================== OVERLAY =================================
        void updateOverlay(float dt);
        bool handleOverlayInput(GLFWwindow *window);
        bool isOverlayVisible() const;
        bool overlayReadyToFly() const;
        ui::CompletionChoice overlayChoice() const;
        void showBriefing(const mission::MissionDefinition &mission);
        void showCompletionPrompt(const mission::MissionRuntime &runtime);
        void hideOverlay();
        void resetOverlay();
        void renderOverlay();

        // ============================== HUD =====================================
        hud::FlightHUD &hud() { return *hud_; }
        void updateHUD(const flight::FlightData &data);
        void renderHUD();

    private:
        // Owners of each interface layer
        std::unique_ptr<hud::FlightHUD> hud_;
        std::unique_ptr<ui::MissionMenu> menu_;
        std::unique_ptr<ui::MissionPlanner> planner_;
        std::unique_ptr<ui::MissionOverlay> overlay_;

        // External dependencies and layout parameters
        mission::MissionRegistry *registry_ = nullptr;
        int screenWidth_ = 1280;
        int screenHeight_ = 720;
    };

} // namespace ui
