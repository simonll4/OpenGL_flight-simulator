/**
 * @file MissionOverlay.h
 * @brief Overlay system for mission briefing and completion
 *
 * Renders modal messages during Briefing and Completed phases:
 * - Briefing: Shows title, objectives, and start prompt
 * - Completion: Shows summary and options (menu or free flight)
 */

#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"
#include "../../gfx/rendering/TrueTypeFont.h"

// Forward declarations to avoid includes
struct GLFWwindow;

namespace mission
{
    struct MissionDefinition;
    class MissionRuntime;
}

namespace ui
{

    /**
     * @brief Mission completion options
     */
    enum class CompletionChoice
    {
        None,         // Not yet chosen
        ReturnToMenu, // Return to menu (ENTER)
        FreeFlight    // Continue in free flight (SPACE)
    };

    /**
     * @brief Modal overlay for missions
     */
    class MissionOverlay
    {
    public:
        MissionOverlay();
        ~MissionOverlay() = default;

        /**
         * @brief Initializes graphics resources
         */
        void init(int screenWidth, int screenHeight);
        void setScreenSize(int screenWidth, int screenHeight);

        /**
         * @brief Shows the briefing of a mission
         */
        void showBriefing(const mission::MissionDefinition &mission);

        /**
         * @brief Shows the completion prompt
         */
        void showCompletionPrompt(const mission::MissionRuntime &runtime);

        /**
         * @brief Hides the overlay
         */
        void hide();

        /**
         * @brief Renders the overlay
         */
        void render();

        /**
         * @brief Updates animations
         */
        void update(float dt);

        /**
         * @brief Processes user input
         * @return true if any action was processed
         */
        bool handleInput(GLFWwindow *window);

        /**
         * @brief Checks if the pilot is ready to fly (briefing)
         */
        bool readyToFly() const { return readyToFly_; }

        /**
         * @brief Checks if the overlay is visible
         */
        bool isVisible() const { return visible_; }

        /**
         * @brief Gets user choice on completion screen
         */
        CompletionChoice getCompletionChoice() const { return completionChoice_; }

        /**
         * @brief Resets the overlay state
         */
        void reset();

    private:
        // Screen dimensions
        int screenWidth_;
        int screenHeight_;
        bool rendererInitialized_ = false;
        gfx::Renderer2D renderer_;

        // State
        bool visible_;
        bool showingBriefing_;
        bool showingCompletion_;
        bool readyToFly_;
        CompletionChoice completionChoice_;
        int selectedOption_; // 0 = menu, 1 = free flight

        // Content
        std::string missionName_;
        std::string briefingText_;
        std::string metricsText_;

        // Animation
        float fadeAlpha_;
        float blinkTimer_;

        // Input control
        bool enterKeyWasPressed_;
        bool spaceKeyWasPressed_;
        bool tabKeyWasPressed_;
        bool upKeyWasPressed_;
        bool downKeyWasPressed_;

        // Helpers
        void renderBriefing();
        void renderCompletion();
        void drawBackground(float alpha);
        void drawBox(float x, float y, float w, float h);
        std::string formatMetrics(const mission::MissionRuntime &runtime);
        std::vector<std::string> splitLines(const std::string &text) const;
        glm::vec2 overlayTextSize(const std::string &text, float size, float lineSpacing = 1.0f) const;
        void drawOverlayText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                             const glm::vec4 &color, const glm::vec2 &anchor = glm::vec2(0.5f),
                             float lineSpacing = 1.0f);

        gfx::TrueTypeFont overlayFont_;
        bool overlayFontReady_ = false;
    };

} // namespace ui
