/**
 * @file MissionPlanner.h
 * @brief Mission editor and waypoint planner.
 *
 * This class provides an interactive user interface for defining the flight
 * path the aircraft will follow in the simulation. The interface consists of
 * three panels: an orthographic map where waypoints are inserted and dragged,
 * an altitude vs. distance graph to visualize the vertical profile and adjust
 * heights, and a storyboard of cards summarizing each mission segment.
 *
 * The planner handles converting world positions (XZ coordinates) to screen
 * coordinates, managing input events (mouse and keyboard), and updating the
 * MissionDefinition structure with user changes. The goal is to offer a simple
 * yet professional tool for configuring missions before launching the flight
 * simulator.
 */

#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "../../mission/MissionDefinition.h"
#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"
#include "../../gfx/rendering/TrueTypeFont.h"

// Forward declaration of GLFWwindow struct to avoid including GLFW in header
struct GLFWwindow;

namespace ui
{

    /**
     * @brief Results returned by the planner.
     *
     * When the user interacts with the interface, they can request to start
     * the mission (startRequested) or cancel and return to the main menu
     * (cancelRequested). These flags are consumed by the application's main loop.
     */
    struct PlannerResult
    {
        bool startRequested = false;
        bool cancelRequested = false;
    };

    /**
     * @brief Mission planner with graphical waypoint editing.
     *
     * The MissionPlanner class encapsulates all logic for the planning screen.
     * It uses a shared Renderer2D to draw the interface and a MissionDefinition
     * as storage for the current mission under construction. The user can add,
     * move, and remove waypoints on the map, adjust their altitudes via keyboard
     * commands, and visualize the altitude profile and total distance.
     */
    class MissionPlanner
    {
    public:
        MissionPlanner();

        /**
         * @brief Initializes the planner.
         * @param screenWidth Screen width in pixels.
         * @param screenHeight Screen height in pixels.
         * @param sharedRenderer Pointer to an already initialized Renderer2D.
         */
        void init(int screenWidth, int screenHeight, gfx::Renderer2D *sharedRenderer);

        /**
         * @brief Adjusts screen size.
         *
         * Should be called when the window is resized to recalculate layouts and scaling.
         */
        void setScreenSize(int width, int height);

        /**
         * @brief Loads an existing mission for editing.
         * @param mission Mission definition to edit.
         *
         * The supplied mission is copied into the planner, and the map framing
         * is automatically calculated based on the start position and existing
         * waypoints. All interaction states are also reset.
         */
        void loadMission(const mission::MissionDefinition &mission);

        /**
         * @brief Updates planner state.
         * @param window Pointer to GLFW window for reading inputs.
         * @param deltaTime Time in seconds elapsed since last frame.
         */
        void update(GLFWwindow *window, float deltaTime);

        /**
         * @brief Draws the planner interface.
         */
        void render();

        /**
         * @brief Returns the planner interaction state.
         */
        PlannerResult getResult() const { return result_; }

        /**
         * @brief Gets a constant reference to the mission being edited.
         */
        const mission::MissionDefinition &getMission() const { return workingMission_; }

        /**
         * @brief Resets the planner to a neutral state.
         *
         * Clears start/cancel flags and cancels ongoing interactions.
         * Does not remove or alter waypoints.
         */
        void reset();

        // === Additional Functionality ===
        /**
         * @brief Generates a mission report (not implemented).
         */
        void generateMissionReport();

        /**
         * @brief Automatically generates a waypoint pattern.
         * @param pattern Pattern selection (0: square, 1: circle, etc.).
         */
        void autoGenerateWaypoints(int pattern);

        /**
         * @brief Validates if the mission meets minimum requirements.
         * @return true if at least one waypoint is defined.
         */
        bool validateMission() const;

        /**
         * @brief Saves the mission to a JSON file (not implemented).
         */
        void saveMissionToFile(const std::string &path);

        /**
         * @brief Loads a mission from a JSON file (not implemented).
         */
        void loadMissionFromFile(const std::string &path);

    private:
        // === Internal Methods ===
        void updateLayout();
        void handleKeyboardInput(GLFWwindow *window, float deltaTime);
        void handleMouseInput(GLFWwindow *window);
        void updateStoryboardHover();
        void renderBackground();
        void renderToolbar();
        void renderMap();
        void renderProfile();
        void renderStoryboard();
        void renderInstructions();
        void renderStartButton();
        void drawPlannerText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                             const glm::vec4 &color, const glm::vec2 &anchor = glm::vec2(0.5f),
                             float lineSpacing = 1.0f);
        glm::vec2 plannerTextSize(const std::string &text, float size, float lineSpacing = 1.0f) const;

        glm::vec2 mapWorldToScreen(const glm::vec3 &world) const;
        glm::vec3 mapScreenToWorld(const glm::vec2 &screen) const;
        int findWaypointNear(const glm::vec2 &screen, float threshold) const;
        size_t maxVisibleCards() const;
        bool cardRect(size_t index, glm::vec2 &pos, glm::vec2 &size) const;
        bool cursorInsideMap() const;
        bool cursorInsideProfile() const;
        bool cursorInsideStoryboard() const;
        float computeMissionLength() const;

        // === Members ===
        gfx::Renderer2D *renderer_ = nullptr;       ///< Shared 2D renderer
        mission::MissionDefinition workingMission_; ///< Mission under construction
        PlannerResult result_;                      ///< Result of the last interaction

        int screenWidth_ = 1280; ///< Current screen width
        int screenHeight_ = 720; ///< Current screen height

        // Interface rectangles
        glm::vec2 mapOrigin_ = glm::vec2(0.0f);
        glm::vec2 mapSize_ = glm::vec2(1.0f);
        glm::vec2 profileOrigin_ = glm::vec2(0.0f);
        glm::vec2 profileSize_ = glm::vec2(1.0f);
        glm::vec2 storyboardOrigin_ = glm::vec2(0.0f);
        glm::vec2 storyboardSize_ = glm::vec2(1.0f);
        glm::vec2 buttonPos_ = glm::vec2(0.0f);
        glm::vec2 buttonSize_ = glm::vec2(220.0f, 56.0f);
        glm::vec2 cursorPos_ = glm::vec2(0.0f);

        // Navigation and scale variables
        glm::vec2 mapCenter_ = glm::vec2(0.0f); ///< Center point in world represented on screen
        float mapHalfExtent_ = 10000.0f;        ///< Maximum visible extent on each axis (meters)
        float defaultAltitude_ = 1500.0f;       ///< Default altitude for new waypoints

        int selectedIndex_ = -1;  ///< Index of waypoint selected for editing
        bool draggingXY_ = false; ///< Waypoint is being dragged in XZ plane
        int draggingIndex_ = -1;  ///< Index of waypoint being dragged

        // Button/key state flags
        bool leftMouseHeld_ = false;
        bool rightMouseHeld_ = false;
        bool enterHeld_ = false;
        bool escHeld_ = false;
        bool rHeld_ = false;
        bool fHeld_ = false;

        // Hover and selection states
        bool startButtonHovered_ = false;
        int hoveredCardIndex_ = -1;

        // Render caches
        float cachedMissionLength_ = 0.0f;
        float storyboardCardHeight_ = 90.0f;
        float panSpeed_ = 200.0f; ///< Map panning speed in meters/second

        gfx::TrueTypeFont plannerFont_;
        bool plannerFontReady_ = false;
    };

} // namespace ui
