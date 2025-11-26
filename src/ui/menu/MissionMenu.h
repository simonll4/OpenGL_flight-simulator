/**
 * @file MissionMenu.h
 * @brief Mission selection screen.
 */

#pragma once

#include "../../mission/MissionDefinition.h"
#include "../../mission/MissionRegistry.h"
#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"
#include "../../gfx/rendering/TrueTypeFont.h"
#include <memory>

extern "C"
{
#include <GLFW/glfw3.h>
}

namespace ui
{

    /**
     * @brief Result of user selection in the menu.
     */
    struct MenuResult
    {
        bool missionSelected = false;
        bool exitRequested = false;
        int selectedMissionIndex = -1;
    };

    /**
     * @brief Welcome menu and mission selection.
     *
     * Displays a list of available missions and allows the user to
     * select one to start or exit the simulator.
     */
    class MissionMenu
    {
    public:
        MissionMenu();
        ~MissionMenu() = default;

        /**
         * @brief Initialize the menu.
         * @param registry Registry of available missions.
         * @param screenWidth Screen width.
         * @param screenHeight Screen height.
         */
        void init(mission::MissionRegistry *registry, int screenWidth, int screenHeight);

        /**
         * @brief Update menu state (input handling).
         * @param window GLFW window for capturing input.
         * @param deltaTime Time elapsed since last frame.
         */
        void update(GLFWwindow *window, float deltaTime);

        /**
         * @brief Render the menu.
         */
        void render();

        /**
         * @brief Get selection result.
         */
        MenuResult getResult() const;

        /**
         * @brief Reset the menu to show it again.
         */
        void reset();

        /**
         * @brief Update screen size.
         */
        void setScreenSize(int width, int height);

        /**
         * @brief Preselect a mission by index.
         */
        void preselectMission(int index);

        /**
         * @brief Access to 2D renderer for reuse in other views.
         */
        gfx::Renderer2D *getRenderer() { return &renderer2D_; }

    private:
        mission::MissionRegistry *registry_;
        gfx::Renderer2D renderer2D_;

        int screenWidth_;
        int screenHeight_;
        int selectedIndex_;
        MenuResult result_;

        // Key state to avoid repetition
        bool enterKeyPressed_;
        bool escKeyPressed_;
        bool upKeyPressed_;
        bool downKeyPressed_;

        // Helpers
        void handleInput(GLFWwindow *window);
        void renderBackground();
        void renderTitle();
        void renderMissionList();
        void renderMissionDetails();
        void renderInstructions();
        void drawMenuText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                          const glm::vec4 &color, const glm::vec2 &anchor = glm::vec2(0.5f),
                          float lineSpacing = 1.0f);
        glm::vec2 menuTextSize(const std::string &text, float size, float lineSpacing = 1.0f) const;

        gfx::TrueTypeFont menuFont_;
        bool menuFontReady_ = false;
    };

} // namespace ui
