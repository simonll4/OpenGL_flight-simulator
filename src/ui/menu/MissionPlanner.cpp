/**
 * @file MissionPlanner.cpp
 * @brief Implementation of the mission editor and waypoint planner.
 */

#include "MissionPlanner.h"

// Include GLFW only in the source to avoid polluting the header
extern "C"
{
#include <GLFW/glfw3.h>
}

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>

extern "C"
{
#include <glad/glad.h>
}

namespace ui
{

    // -----------------------------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------------------------
    MissionPlanner::MissionPlanner()
        : renderer_(nullptr),
          selectedIndex_(-1),
          draggingXY_(false),
          draggingIndex_(-1),
          leftMouseHeld_(false),
          rightMouseHeld_(false),
          enterHeld_(false),
          escHeld_(false),
          rHeld_(false),
          fHeld_(false),
          startButtonHovered_(false),
          hoveredCardIndex_(-1),
          cachedMissionLength_(0.0f),
          storyboardCardHeight_(90.0f),
          panSpeed_(200.0f)
    {
    }

    // -----------------------------------------------------------------------------
    // Initialization
    // -----------------------------------------------------------------------------
    void MissionPlanner::init(int screenWidth, int screenHeight, gfx::Renderer2D *sharedRenderer)
    {
        renderer_ = sharedRenderer;
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;
        updateLayout();
        // The shared renderer should already be initialized elsewhere
        const std::string fontPath = "assets/fonts/RobotoMono-Regular.ttf";
        // Use larger base size (96px) for better quality when scaling
        plannerFontReady_ = plannerFont_.loadFromFile(fontPath, 96.0f, 2048);
        if (!plannerFontReady_)
        {
            std::cerr << "[MissionPlanner] Could not load RobotoMono font at " << fontPath << std::endl;
        }
    }

    void MissionPlanner::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        updateLayout();
        if (renderer_)
        {
            renderer_->setScreenSize(width, height);
        }
    }

    // -----------------------------------------------------------------------------
    // Load Mission
    // -----------------------------------------------------------------------------
    void MissionPlanner::loadMission(const mission::MissionDefinition &mission)
    {
        workingMission_ = mission;
        result_ = PlannerResult{};
        selectedIndex_ = -1;
        draggingXY_ = false;
        draggingIndex_ = -1;
        leftMouseHeld_ = false;
        rightMouseHeld_ = false;
        enterHeld_ = false;
        escHeld_ = false;
        rHeld_ = false;
        fHeld_ = false;
        startButtonHovered_ = false;
        hoveredCardIndex_ = -1;
        // Default altitude is that of the start point
        defaultAltitude_ = mission.startPosition.y;
        // Center map on start position
        mapCenter_ = glm::vec2(mission.startPosition.x, mission.startPosition.z);
        // Calculate visible extent based on furthest waypoint
        float maxRadius = 0.0f;
        auto accumulateRadius = [&](const glm::vec3 &pos)
        {
            glm::vec2 delta(pos.x - mapCenter_.x, pos.z - mapCenter_.y);
            maxRadius = std::max(maxRadius, glm::length(delta));
        };
        accumulateRadius(mission.startPosition);
        for (const auto &wp : mission.waypoints)
        {
            accumulateRadius(wp.position);
        }
        // Define a minimum so the map doesn't get too small
        mapHalfExtent_ = std::max(2000.0f, maxRadius * 1.5f + 500.0f);
        // Precalculate length
        cachedMissionLength_ = computeMissionLength();
    }

    // -----------------------------------------------------------------------------
    // Reset
    // -----------------------------------------------------------------------------
    void MissionPlanner::reset()
    {
        result_ = PlannerResult{};
        draggingXY_ = false;
        draggingIndex_ = -1;
        leftMouseHeld_ = false;
        rightMouseHeld_ = false;
        enterHeld_ = false;
        escHeld_ = false;
        rHeld_ = false;
        fHeld_ = false;
        startButtonHovered_ = false;
        hoveredCardIndex_ = -1;
    }

    // -----------------------------------------------------------------------------
    // Update per frame
    // -----------------------------------------------------------------------------
    void MissionPlanner::update(GLFWwindow *window, float deltaTime)
    {
        if (!renderer_)
        {
            return;
        }
        // Update cursor position
        double mx = 0.0;
        double my = 0.0;
        glfwGetCursorPos(window, &mx, &my);
        cursorPos_ = glm::vec2(static_cast<float>(mx), static_cast<float>(my));
        // Process keyboard and mouse
        handleKeyboardInput(window, deltaTime);
        handleMouseInput(window);
        // If dragging a waypoint in XZ, update its position
        if (draggingXY_ && leftMouseHeld_ && draggingIndex_ >= 0 && draggingIndex_ < static_cast<int>(workingMission_.waypoints.size()))
        {
            glm::vec3 world = mapScreenToWorld(cursorPos_);
            world.y = workingMission_.waypoints[draggingIndex_].position.y;
            workingMission_.waypoints[draggingIndex_].position = world;
        }
        updateStoryboardHover();
        // Recalculate length to show updated value
        cachedMissionLength_ = computeMissionLength();
    }

    // -----------------------------------------------------------------------------
    // Full Render
    // -----------------------------------------------------------------------------
    void MissionPlanner::render()
    {
        if (!renderer_)
        {
            return;
        }
        // Draw all elements
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        renderer_->begin();
        renderBackground();
        renderToolbar();
        renderMap();
        renderProfile();
        renderStoryboard();
        renderInstructions();
        renderStartButton();
        renderer_->end();
        glDisable(GL_BLEND);
        // flush() is already called by end() internally
    }

    // -----------------------------------------------------------------------------
    // Panel Layout
    // -----------------------------------------------------------------------------
    void MissionPlanner::updateLayout()
    {
        float sideMargin = screenWidth_ * 0.05f;
        float topMargin = screenHeight_ * 0.10f;
        // Map area (55% width, 55% height)
        mapOrigin_ = glm::vec2(sideMargin, topMargin);
        mapSize_ = glm::vec2(screenWidth_ * 0.55f, screenHeight_ * 0.55f);
        // Profile area (18% height)
        float profileTop = mapOrigin_.y + mapSize_.y + 30.0f;
        profileOrigin_ = glm::vec2(sideMargin, profileTop);
        profileSize_ = glm::vec2(mapSize_.x, screenHeight_ * 0.18f);
        // Storyboard area to the right
        float storyboardX = mapOrigin_.x + mapSize_.x + sideMargin;
        float storyboardWidth = screenWidth_ - storyboardX - sideMargin;
        float storyboardHeight = (profileOrigin_.y + profileSize_.y) - mapOrigin_.y;
        // Ensure minimum width for storyboard column
        storyboardOrigin_ = glm::vec2(storyboardX, mapOrigin_.y);
        storyboardSize_ = glm::vec2(std::max(300.0f, storyboardWidth), storyboardHeight);
        // Start button below the list
        buttonSize_ = glm::vec2(std::max(220.0f, storyboardSize_.x * 0.65f), 56.0f);
        buttonPos_ = glm::vec2(storyboardOrigin_.x + (storyboardSize_.x - buttonSize_.x) * 0.5f,
                               storyboardOrigin_.y + storyboardSize_.y - buttonSize_.y - 14.0f);
    }

    // -----------------------------------------------------------------------------
    // Keyboard Processing
    // -----------------------------------------------------------------------------
    void MissionPlanner::handleKeyboardInput(GLFWwindow *window, float deltaTime)
    {
        // Map panning proportional to visible extent
        float move = panSpeed_ * deltaTime;
        float factor = mapHalfExtent_ / 5000.0f; // adjust sensitivity based on scale
        float delta = move * factor;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            mapCenter_.x -= delta;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            mapCenter_.x += delta;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            mapCenter_.y -= delta;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            mapCenter_.y += delta;
        }
        // Confirm start with ENTER
        bool enterDown = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
                          glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS);
        if (enterDown && !enterHeld_)
        {
            if (validateMission())
            {
                result_.startRequested = true;
            }
            enterHeld_ = true;
        }
        else if (!enterDown)
        {
            enterHeld_ = false;
        }
        // Cancel with ESC
        bool escDown = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
        if (escDown && !escHeld_)
        {
            result_.cancelRequested = true;
            escHeld_ = true;
        }
        else if (!escDown)
        {
            escHeld_ = false;
        }
        // Increase altitude of selected waypoint with R
        bool rDown = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
        if (rDown && !rHeld_ && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(workingMission_.waypoints.size()))
        {
            workingMission_.waypoints[selectedIndex_].position.y += 100.0f;
            rHeld_ = true;
        }
        else if (!rDown)
        {
            rHeld_ = false;
        }
        // Decrease altitude with F
        bool fDown = (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS);
        if (fDown && !fHeld_ && selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(workingMission_.waypoints.size()))
        {
            float &alt = workingMission_.waypoints[selectedIndex_].position.y;
            alt = std::max(0.0f, alt - 100.0f);
            fHeld_ = true;
        }
        else if (!fDown)
        {
            fHeld_ = false;
        }
    }

    // -----------------------------------------------------------------------------
    // Mouse Processing
    // -----------------------------------------------------------------------------
    void MissionPlanner::handleMouseInput(GLFWwindow *window)
    {
        // Detect button state
        bool leftDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
        bool rightDown = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
        // Update hover flag on start button
        startButtonHovered_ = (cursorPos_.x >= buttonPos_.x && cursorPos_.x <= (buttonPos_.x + buttonSize_.x) &&
                               cursorPos_.y >= buttonPos_.y && cursorPos_.y <= (buttonPos_.y + buttonSize_.y));
        // Left click just pressed
        if (leftDown && !leftMouseHeld_)
        {
            // If cursor is over button and mission is valid -> start
            if (startButtonHovered_ && validateMission())
            {
                result_.startRequested = true;
            }
            else if (cursorInsideMap())
            {
                // Check if there is a waypoint nearby to drag
                int hitIndex = findWaypointNear(cursorPos_, 14.0f);
                if (hitIndex != -1)
                {
                    draggingXY_ = true;
                    draggingIndex_ = hitIndex;
                    selectedIndex_ = hitIndex;
                }
                else
                {
                    // Add new waypoint at cursor position
                    mission::WaypointDef wp;
                    wp.position = mapScreenToWorld(cursorPos_);
                    wp.name = "WP" + std::to_string(static_cast<int>(workingMission_.waypoints.size() + 1));
                    workingMission_.waypoints.push_back(wp);
                    selectedIndex_ = static_cast<int>(workingMission_.waypoints.size()) - 1;
                }
            }
            else if (cursorInsideStoryboard())
            {
                // Select waypoint card
                size_t visible = std::min(maxVisibleCards(), workingMission_.waypoints.size());
                for (size_t i = 0; i < visible; ++i)
                {
                    glm::vec2 pos;
                    glm::vec2 size;
                    if (!cardRect(i, pos, size))
                    {
                        continue;
                    }
                    if (cursorPos_.x >= pos.x && cursorPos_.x <= pos.x + size.x &&
                        cursorPos_.y >= pos.y && cursorPos_.y <= pos.y + size.y)
                    {
                        selectedIndex_ = static_cast<int>(i);
                        break;
                    }
                }
            }
        }
        // Releasing left button ends drag
        else if (!leftDown && leftMouseHeld_)
        {
            draggingXY_ = false;
            draggingIndex_ = -1;
        }
        leftMouseHeld_ = leftDown;
        // Right click just pressed to delete
        if (rightDown && !rightMouseHeld_)
        {
            if (cursorInsideMap())
            {
                int index = findWaypointNear(cursorPos_, 16.0f);
                if (index >= 0 && index < static_cast<int>(workingMission_.waypoints.size()))
                {
                    workingMission_.waypoints.erase(workingMission_.waypoints.begin() + index);
                    if (selectedIndex_ == index)
                    {
                        selectedIndex_ = -1;
                    }
                    if (draggingIndex_ == index)
                    {
                        draggingXY_ = false;
                        draggingIndex_ = -1;
                    }
                }
            }
        }
        rightMouseHeld_ = rightDown;
    }

    // -----------------------------------------------------------------------------
    // Update storyboard hover
    // -----------------------------------------------------------------------------
    void MissionPlanner::updateStoryboardHover()
    {
        hoveredCardIndex_ = -1;
        if (!cursorInsideStoryboard())
        {
            return;
        }
        size_t visible = std::min(maxVisibleCards(), workingMission_.waypoints.size());
        for (size_t i = 0; i < visible; ++i)
        {
            glm::vec2 pos;
            glm::vec2 size;
            if (!cardRect(i, pos, size))
            {
                continue;
            }
            if (cursorPos_.x >= pos.x && cursorPos_.x <= pos.x + size.x &&
                cursorPos_.y >= pos.y && cursorPos_.y <= pos.y + size.y)
            {
                hoveredCardIndex_ = static_cast<int>(i);
                break;
            }
        }
    }

    // -----------------------------------------------------------------------------
    // Render background and headers
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderBackground()
    {
        // General background
        glm::vec4 baseColor(0.02f, 0.03f, 0.05f, 1.0f);
        renderer_->drawRect(glm::vec2(0.0f), glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)), baseColor, true);

        // Top strip with visual gradient
        glm::vec4 headerColor(0.025f, 0.045f, 0.07f, 1.0f);
        float headerHeight = screenHeight_ * 0.08f;
        renderer_->drawRect(glm::vec2(0.0f, 0.0f), glm::vec2(static_cast<float>(screenWidth_), headerHeight), headerColor, true);

        // Decorative bottom line of header
        glm::vec4 accentLine(0.15f, 0.55f, 0.85f, 0.8f);
        renderer_->drawRect(glm::vec2(0.0f, headerHeight - 2.0f), glm::vec2(static_cast<float>(screenWidth_), 2.0f), accentLine, true);

        // Main title vertically centered in header
        glm::vec2 titlePos(screenWidth_ * 0.5f, headerHeight * 0.38f);
        glm::vec2 titleShadow(2.0f, 2.0f);
        // Title shadow
        drawPlannerText("MISSION PLANNER", titlePos + titleShadow, 32.0f,
                        glm::vec4(0.0f, 0.0f, 0.0f, 0.5f), glm::vec2(0.5f, 0.5f));
        // Main title with bright cyan color
        drawPlannerText("MISSION PLANNER", titlePos,
                        32.0f, glm::vec4(0.55f, 0.92f, 1.0f, 1.0f), glm::vec2(0.5f, 0.5f));
    }

    // -----------------------------------------------------------------------------
    // Render toolbar
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderToolbar()
    {
        // Bar position and size (above map and storyboard)
        glm::vec2 pos(mapOrigin_.x, mapOrigin_.y - 36.0f);
        float width = (storyboardOrigin_.x + storyboardSize_.x) - mapOrigin_.x;
        glm::vec2 size(width, 30.0f);
        glm::vec4 barBg(0.03f, 0.05f, 0.08f, 0.95f);
        glm::vec4 barBorder(0.15f, 0.35f, 0.55f, 0.9f);
        renderer_->drawRect(pos, size, barBg, true);
        renderer_->drawRect(pos, size, barBorder, false);
        // Build texts
        auto formatFloat = [](float value, int decimals)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(decimals) << value;
            return oss.str();
        };
        std::string left = "Waypoints: " + std::to_string(workingMission_.waypoints.size());
        float totalKm = cachedMissionLength_ > 0.0f ? cachedMissionLength_ / 1000.0f : 0.0f;
        std::string center = "Length: " + formatFloat(totalKm, 1) + " km";
        std::string right = "Wind " + formatFloat(workingMission_.environment.windSpeed, 1) + " m/s  @" +
                            formatFloat(workingMission_.environment.windDirection, 0) + "°";
        // Draw texts with alignment
        drawPlannerText(left, pos + glm::vec2(18.0f, size.y * 0.5f), 16.0f,
                        glm::vec4(0.75f, 0.85f, 1.0f, 1.0f), glm::vec2(0.0f, 0.5f));
        drawPlannerText(center, pos + glm::vec2(size.x * 0.5f, size.y * 0.5f), 16.0f,
                        glm::vec4(0.6f, 0.9f, 0.8f, 1.0f), glm::vec2(0.5f, 0.5f));
        drawPlannerText(right, pos + glm::vec2(size.x - 18.0f, size.y * 0.5f), 16.0f,
                        glm::vec4(0.8f, 0.8f, 0.95f, 1.0f), glm::vec2(1.0f, 0.5f));
    }

    // -----------------------------------------------------------------------------
    // Render map
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderMap()
    {
        // Map background and border
        glm::vec4 mapBg(0.05f, 0.08f, 0.12f, 0.95f);
        glm::vec4 mapBorder(0.18f, 0.45f, 0.75f, 1.0f);
        renderer_->drawRect(mapOrigin_, mapSize_, mapBg, true);
        renderer_->drawRect(mapOrigin_, mapSize_, mapBorder, false);
        // Draw basic grid (9 vertical and horizontal lines)
        glm::vec4 gridColor(0.1f, 0.25f, 0.35f, 0.4f);
        for (int i = -4; i <= 4; ++i)
        {
            float t = static_cast<float>(i) / 4.0f;
            float x = mapOrigin_.x + mapSize_.x * 0.5f + t * (mapSize_.x * 0.5f);
            renderer_->drawLine(glm::vec2(x, mapOrigin_.y), glm::vec2(x, mapOrigin_.y + mapSize_.y), gridColor, 1.0f);
            float y = mapOrigin_.y + mapSize_.y * 0.5f + t * (mapSize_.y * 0.5f);
            renderer_->drawLine(glm::vec2(mapOrigin_.x, y), glm::vec2(mapOrigin_.x + mapSize_.x, y), gridColor, 1.0f);
        }
        // Draw connections between nodes
        glm::vec3 prevWorld = workingMission_.startPosition;
        glm::vec2 prevScreen = mapWorldToScreen(prevWorld);
        for (const auto &wp : workingMission_.waypoints)
        {
            glm::vec2 curScreen = mapWorldToScreen(wp.position);
            renderer_->drawLine(prevScreen, curScreen, glm::vec4(0.25f, 0.6f, 0.9f, 0.8f), 2.5f);
            prevScreen = curScreen;
            prevWorld = wp.position;
        }
        // Origin marker (start point)
        glm::vec2 startScreen = mapWorldToScreen(workingMission_.startPosition);
        glm::vec4 startColor(0.15f, 0.9f, 0.4f, 1.0f);
        renderer_->drawCircle(startScreen, 10.0f, startColor, 32, true);
        renderer_->drawCircle(startScreen, 12.0f, startColor, 32, false);
        drawPlannerText("START", startScreen + glm::vec2(0.0f, -18.0f), 15.0f,
                        glm::vec4(0.75f, 0.9f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));
        // Help message if no waypoints
        if (workingMission_.waypoints.empty())
        {
            glm::vec2 hintPos(mapOrigin_.x + mapSize_.x * 0.5f, mapOrigin_.y + mapSize_.y * 0.55f);
            drawPlannerText("Click on map to insert first waypoint", hintPos,
                            18.0f, glm::vec4(0.85f, 0.9f, 1.0f, 0.9f), glm::vec2(0.5f, 0.5f));
        }
        // Draw each waypoint
        for (size_t i = 0; i < workingMission_.waypoints.size(); ++i)
        {
            const auto &wp = workingMission_.waypoints[i];
            glm::vec2 p = mapWorldToScreen(wp.position);
            bool active = (static_cast<int>(i) == selectedIndex_);
            bool hovered = (static_cast<int>(i) == hoveredCardIndex_);
            glm::vec4 baseColor = active    ? glm::vec4(0.95f, 0.4f, 0.6f, 1.0f)
                                  : hovered ? glm::vec4(0.35f, 0.75f, 1.0f, 1.0f)
                                            : glm::vec4(0.25f, 0.6f, 0.9f, 1.0f);
            float innerRadius = active ? 7.5f : 6.0f;
            float outerRadius = innerRadius + 2.0f;
            renderer_->drawCircle(p, outerRadius, glm::vec4(baseColor.r, baseColor.g, baseColor.b, 0.3f), 32, false);
            renderer_->drawCircle(p, innerRadius, baseColor, 32, true);
            // Labels
            std::string label = "WP" + std::to_string(static_cast<int>(i + 1));
            drawPlannerText(label, p + glm::vec2(0.0f, -12.0f), 16.0f,
                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.5f, 1.0f));
            // Altitude below
            std::string altStr = std::to_string(static_cast<int>(std::round(wp.position.y))) + " m";
            drawPlannerText(altStr, p + glm::vec2(0.0f, 12.0f), 13.0f,
                            glm::vec4(0.96f, 0.95f, 0.85f, 1.0f), glm::vec2(0.5f, 0.0f));
        }
    }

    // -----------------------------------------------------------------------------
    // Render altitude profile
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderProfile()
    {
        // Profile background and border
        glm::vec4 bg(0.05f, 0.08f, 0.12f, 0.95f);
        glm::vec4 border(0.18f, 0.45f, 0.75f, 1.0f);
        renderer_->drawRect(profileOrigin_, profileSize_, bg, true);
        renderer_->drawRect(profileOrigin_, profileSize_, border, false);
        // Axes and labels
        glm::vec4 axisColor(0.3f, 0.4f, 0.5f, 0.8f);
        renderer_->drawLine(profileOrigin_ + glm::vec2(40.0f, profileSize_.y - 20.0f),
                            profileOrigin_ + glm::vec2(profileSize_.x - 20.0f, profileSize_.y - 20.0f),
                            axisColor, 1.0f);
        renderer_->drawLine(profileOrigin_ + glm::vec2(40.0f, 20.0f),
                            profileOrigin_ + glm::vec2(40.0f, profileSize_.y - 20.0f),
                            axisColor, 1.0f);
        // Collect points (cumulative distance, altitude)
        std::vector<glm::vec2> nodes;
        float totalDist = 0.0f;
        float maxAlt = 3000.0f; // minimum scale
        nodes.push_back(glm::vec2(0.0f, workingMission_.startPosition.y));
        maxAlt = std::max(maxAlt, workingMission_.startPosition.y);
        glm::vec3 prev = workingMission_.startPosition;
        for (const auto &wp : workingMission_.waypoints)
        {
            float d = glm::length(glm::vec2(wp.position.x - prev.x, wp.position.z - prev.z));
            totalDist += d;
            nodes.push_back(glm::vec2(totalDist, wp.position.y));
            maxAlt = std::max(maxAlt, wp.position.y);
            prev = wp.position;
        }
        if (totalDist < 100.0f)
            totalDist = 100.0f;
        maxAlt *= 1.2f; // margin
        // Determine scales
        float scaleX = (profileSize_.x - 80.0f) / totalDist;
        float scaleY = (profileSize_.y - 60.0f) / maxAlt;
        float originX = profileOrigin_.x + 40.0f;
        float originY = profileOrigin_.y + profileSize_.y - 20.0f;
        // Draw area chart
        if (nodes.size() >= 2)
        {
            for (size_t i = 0; i < nodes.size() - 1; ++i)
            {
                glm::vec2 p1 = nodes[i];
                glm::vec2 p2 = nodes[i + 1];
                float x1 = originX + p1.x * scaleX;
                float y1 = originY - p1.y * scaleY;
                float x2 = originX + p2.x * scaleX;
                float y2 = originY - p2.y * scaleY;
                // Fill under curve (simplified as quads)
                glm::vec4 fillColor(0.2f, 0.5f, 0.4f, 0.3f);
                // ... (implementation of fill omitted for brevity, using lines for now)
                renderer_->drawLine(glm::vec2(x1, y1), glm::vec2(x2, y2), glm::vec4(0.4f, 0.9f, 0.6f, 0.9f), 2.0f);
            }
        }
        // Draw top line
        // (Already done in loop above)
        // Draw nodes and labels
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            // Calculate current point
            float x = originX + nodes[i].x * scaleX;
            float y = originY - nodes[i].y * scaleY;
            glm::vec2 currentPt(x, y);
            bool active = (static_cast<int>(i - 1) == selectedIndex_); // i=0 is start
            glm::vec4 color = active ? glm::vec4(1.0f, 0.8f, 0.2f, 1.0f) : glm::vec4(0.4f, 0.9f, 0.6f, 1.0f);
            renderer_->drawCircle(currentPt, 5.0f, color, 32, true);
        }
    }

    // Renderizar storyboard de tarjetas
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderStoryboard()
    {
        // Panel de fondo y borde
        glm::vec4 panelBg(0.045f, 0.065f, 0.095f, 0.97f);
        glm::vec4 panelBorder(0.2f, 0.5f, 0.9f, 1.0f);
        renderer_->drawRect(storyboardOrigin_, storyboardSize_, panelBg, true);
        renderer_->drawRect(storyboardOrigin_, storyboardSize_, panelBorder, false);
        // Encabezados
        glm::vec2 headerPos = storyboardOrigin_ + glm::vec2(storyboardSize_.x * 0.5f, 28.0f);
        drawPlannerText("STORYBOARD", headerPos, 22.0f,
                        glm::vec4(0.55f, 0.95f, 1.0f, 1.0f), glm::vec2(0.5f, 0.5f));
        // Subtítulo con información general
        std::string missionName = workingMission_.name.empty() ? "Misión sin nombre" : workingMission_.name;
        std::string overview = missionName + "  |  " + workingMission_.category + "  |  " +
                               workingMission_.environment.weather + "  @ " + workingMission_.environment.timeOfDay;
        drawPlannerText(overview,
                        storyboardOrigin_ + glm::vec2(storyboardSize_.x * 0.5f, 58.0f),
                        17.0f, glm::vec4(0.9f, 0.92f, 0.96f, 1.0f), glm::vec2(0.5f, 0.5f));
        // Calcular tarjetas visibles
        size_t visible = std::min(maxVisibleCards(), workingMission_.waypoints.size());
        if (visible == 0)
        {
            // Si no hay waypoints, no renderizar tarjetas
            return;
        }
        // Distancias acumuladas para métricas por tramo
        std::vector<float> cumulative;
        cumulative.reserve(workingMission_.waypoints.size() + 1);
        cumulative.push_back(0.0f);
        glm::vec2 prevXZ(workingMission_.startPosition.x, workingMission_.startPosition.z);
        for (const auto &wp : workingMission_.waypoints)
        {
            glm::vec2 curXZ(wp.position.x, wp.position.z);
            float d = glm::length(curXZ - prevXZ);
            cumulative.push_back(cumulative.back() + d);
            prevXZ = curXZ;
        }
        float totalDistance = cumulative.back();
        if (totalDistance <= 0.0f)
            totalDistance = 1.0f;
        auto formatFloat = [](float v, int dec)
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(dec) << v;
            return oss.str();
        };

        // Renderizar cada tarjeta visible
        for (size_t i = 0; i < visible; ++i)
        {
            glm::vec2 pos;
            glm::vec2 size;
            if (!cardRect(i, pos, size))
            {
                continue;
            }
            bool isSel = (static_cast<int>(i) == selectedIndex_);
            bool isHover = (static_cast<int>(i) == hoveredCardIndex_);
            glm::vec4 bg = isSel     ? glm::vec4(0.08f, 0.12f, 0.18f, 1.0f)
                           : isHover ? glm::vec4(0.07f, 0.1f, 0.15f, 0.95f)
                                     : glm::vec4(0.06f, 0.09f, 0.14f, 0.9f);
            glm::vec4 accent = isSel     ? glm::vec4(0.4f, 0.95f, 0.8f, 1.0f)
                               : isHover ? glm::vec4(0.35f, 0.8f, 1.0f, 1.0f)
                                         : glm::vec4(0.25f, 0.6f, 0.95f, 1.0f);
            // Fondo y borde de tarjeta
            renderer_->drawRect(pos, size, bg, true);
            renderer_->drawRect(pos, size, glm::vec4(0.12f, 0.3f, 0.45f, 0.8f), false);
            // Franja lateral de color
            renderer_->drawRect(pos, glm::vec2(6.0f, size.y), accent, true);
            // Texto principal: número y nombre
            std::string label = "WP" + std::to_string(static_cast<int>(i + 1)) + "  " +
                                (workingMission_.waypoints[i].name.empty() ? "VECTOR" : workingMission_.waypoints[i].name);
            glm::vec2 labelPos = pos + glm::vec2(size.x * 0.5f, 20.0f);
            glm::vec4 nameColor = isSel ? glm::vec4(1.0f) : glm::vec4(0.92f, 0.96f, 1.0f, 0.96f);
            drawPlannerText(label, labelPos + glm::vec2(1.0f, 1.0f), 20.0f,
                            glm::vec4(0.0f, 0.0f, 0.0f, 0.4f), glm::vec2(0.5f, 0.0f));
            drawPlannerText(label, labelPos, 20.0f,
                            nameColor, glm::vec2(0.5f, 0.0f));
            // Métricas: altitud, distancia del tramo, rumbo
            glm::vec2 prevPosXZ;
            if (i == 0)
            {
                prevPosXZ = glm::vec2(workingMission_.startPosition.x, workingMission_.startPosition.z);
            }
            else
            {
                prevPosXZ = glm::vec2(workingMission_.waypoints[i - 1].position.x, workingMission_.waypoints[i - 1].position.z);
            }
            glm::vec2 curXZ(workingMission_.waypoints[i].position.x, workingMission_.waypoints[i].position.z);
            float legDist = glm::length(curXZ - prevPosXZ);
            float legKm = legDist / 1000.0f;
            float headingRad = std::atan2(curXZ.x - prevPosXZ.x, curXZ.y - prevPosXZ.y);
            float headingDeg = glm::degrees(headingRad);
            if (headingDeg < 0.0f)
                headingDeg += 360.0f;
            std::string metrics = "ALT " + std::to_string(static_cast<int>(std::round(workingMission_.waypoints[i].position.y))) + " m" + "   |   DIST " + formatFloat(legKm, 1) + " km" + "   |   HDG " + std::to_string(static_cast<int>(std::round(headingDeg))) + "°";
            glm::vec2 metricsPos = pos + glm::vec2(size.x * 0.5f, 44.0f);
            drawPlannerText(metrics, metricsPos + glm::vec2(0.8f, 0.8f), 16.5f,
                            glm::vec4(0.0f, 0.0f, 0.0f, 0.35f), glm::vec2(0.5f, 0.0f));
            drawPlannerText(metrics, metricsPos, 16.5f,
                            glm::vec4(0.89f, 0.95f, 1.0f, 1.0f), glm::vec2(0.5f, 0.0f));
            // Línea de progreso relativa a la misión completa
            float progression = cumulative[i + 1] / totalDistance;
            glm::vec2 barPos = pos + glm::vec2(18.0f, size.y - 20.0f);
            glm::vec2 barSize(size.x - 36.0f, 6.0f);
            renderer_->drawRect(barPos, barSize, glm::vec4(0.08f, 0.15f, 0.2f, 1.0f), true);
            renderer_->drawRect(barPos, glm::vec2(barSize.x * progression, barSize.y), accent, true);
        }
        // Aviso de tarjetas ocultas
        size_t total = workingMission_.waypoints.size();
        if (total > visible)
        {
            size_t remaining = total - visible;
            std::string overflow = "+" + std::to_string(remaining) + " waypoints ocultos";
            drawPlannerText(overflow,
                            storyboardOrigin_ + glm::vec2(storyboardSize_.x * 0.5f, storyboardSize_.y - buttonSize_.y - 24.0f),
                            14.0f, glm::vec4(0.9f, 0.6f, 0.2f, 1.0f), glm::vec2(0.5f, 0.5f));
        }
    }

    // -----------------------------------------------------------------------------
    // Render general instructions
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderInstructions()
    {
        std::string instr1 = "MAP: Left click inserts/moves | Right deletes | WASD pans";
        std::string instr2 = "ALTITUDE: R raises | F lowers | ENTER starts mission | ESC returns";
        glm::vec4 instrColor(0.78f, 0.82f, 0.9f, 1.0f);
        drawPlannerText(instr1, glm::vec2(screenWidth_ * 0.5f, screenHeight_ * 0.925f), 18.0f,
                        instrColor, glm::vec2(0.5f, 0.5f));
        drawPlannerText(instr2, glm::vec2(screenWidth_ * 0.5f, screenHeight_ * 0.955f), 18.0f,
                        instrColor, glm::vec2(0.5f, 0.5f));
    }

    // -----------------------------------------------------------------------------
    // Render start button
    // -----------------------------------------------------------------------------
    void MissionPlanner::renderStartButton()
    {
        // Determine colors based on state
        glm::vec4 inactive(0.15f, 0.2f, 0.24f, 0.9f);
        glm::vec4 active(0.1f, 0.55f, 0.35f, 0.95f);
        glm::vec4 hovered(0.2f, 0.85f, 0.5f, 0.95f);
        glm::vec4 color = validateMission() ? active : inactive;
        if (startButtonHovered_ && validateMission())
        {
            color = hovered;
        }
        renderer_->drawRect(buttonPos_, buttonSize_, color, true);
        renderer_->drawRect(buttonPos_, buttonSize_, glm::vec4(0.05f, 0.12f, 0.08f, 1.0f), false);
        // Main text
        std::string label = validateMission() ? "START MISSION" : "ADD WAYPOINTS";
        drawPlannerText(label,
                        buttonPos_ + glm::vec2(buttonSize_.x * 0.5f, buttonSize_.y * 0.38f),
                        22.0f, glm::vec4(1.0f), glm::vec2(0.5f, 0.5f));
        // Secondary text
        std::string sub = validateMission() ? "Validation OK - ready to fly"
                                            : "At least one waypoint needed";
        drawPlannerText(sub,
                        buttonPos_ + glm::vec2(buttonSize_.x * 0.5f, buttonSize_.y * 0.72f),
                        15.0f, glm::vec4(0.85f, 0.95f, 1.0f, 0.9f), glm::vec2(0.5f, 0.5f));
    }

    glm::vec2 MissionPlanner::plannerTextSize(const std::string &text, float size, float lineSpacing) const
    {
        if (plannerFontReady_)
        {
            return plannerFont_.measureText(text, size, lineSpacing);
        }
        float approxWidth = static_cast<float>(text.size()) * size * 0.55f;
        int lines = 1;
        for (char c : text)
        {
            if (c == '\n')
            {
                ++lines;
            }
        }
        float approxHeight = size * lines * lineSpacing;
        return glm::vec2(approxWidth, approxHeight);
    }

    void MissionPlanner::drawPlannerText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                                         const glm::vec4 &color, const glm::vec2 &anchor, float lineSpacing)
    {
        if (!renderer_ || text.empty())
        {
            return;
        }

        glm::vec2 bounds = plannerTextSize(text, size, lineSpacing);
        glm::vec2 origin = anchorPoint - glm::vec2(bounds.x * anchor.x, bounds.y * anchor.y);

        if (plannerFontReady_)
        {
            plannerFont_.drawText(*renderer_, text, origin, size, color, lineSpacing);
        }
        else
        {
            glm::vec2 fallbackCenter = origin + glm::vec2(bounds.x * 0.5f, bounds.y * 0.5f);
            glm::vec2 charSize(size * 0.6f, size);
            float spacing = size * 0.65f;
            gfx::TextRenderer::drawString(*renderer_, text, fallbackCenter, charSize, color, spacing);
        }
    }

    // -----------------------------------------------------------------------------
    // World to screen conversion for map
    // -----------------------------------------------------------------------------
    glm::vec2 MissionPlanner::mapWorldToScreen(const glm::vec3 &world) const
    {
        // Normalize coordinates in range [-1, 1]
        float dx = (world.x - mapCenter_.x) / mapHalfExtent_;
        float dz = (world.z - mapCenter_.y) / mapHalfExtent_;
        // Convert to screen coordinates
        float sx = mapOrigin_.x + mapSize_.x * 0.5f + dx * (mapSize_.x * 0.5f);
        float sy = mapOrigin_.y + mapSize_.y * 0.5f - dz * (mapSize_.y * 0.5f);
        return glm::vec2(sx, sy);
    }

    // -----------------------------------------------------------------------------
    // Screen to world conversion for map
    // -----------------------------------------------------------------------------
    glm::vec3 MissionPlanner::mapScreenToWorld(const glm::vec2 &screen) const
    {
        float nx = (screen.x - (mapOrigin_.x + mapSize_.x * 0.5f)) / (mapSize_.x * 0.5f);
        float nz = ((mapOrigin_.y + mapSize_.y * 0.5f) - screen.y) / (mapSize_.y * 0.5f);
        glm::vec3 world;
        world.x = mapCenter_.x + nx * mapHalfExtent_;
        world.z = mapCenter_.y + nz * mapHalfExtent_;
        world.y = defaultAltitude_;
        return world;
    }

    // -----------------------------------------------------------------------------
    // Find waypoint near screen position
    // -----------------------------------------------------------------------------
    int MissionPlanner::findWaypointNear(const glm::vec2 &screen, float threshold) const
    {
        float best = threshold;
        int index = -1;
        for (size_t i = 0; i < workingMission_.waypoints.size(); ++i)
        {
            glm::vec2 pos = mapWorldToScreen(workingMission_.waypoints[i].position);
            float d = glm::length(pos - screen);
            if (d < best)
            {
                best = d;
                index = static_cast<int>(i);
            }
        }
        return index;
    }

    // -----------------------------------------------------------------------------
    // Determine maximum visible cards
    // -----------------------------------------------------------------------------
    size_t MissionPlanner::maxVisibleCards() const
    {
        float spacing = 12.0f;
        float yStart = storyboardOrigin_.y + 90.0f;
        float yEnd = buttonPos_.y - 24.0f;
        float usable = std::max(0.0f, yEnd - yStart);
        if (usable <= 0.0f)
        {
            return 0;
        }
        float block = storyboardCardHeight_ + spacing;
        int count = static_cast<int>(std::floor((usable + spacing) / block));
        return static_cast<size_t>(std::max(0, count));
    }

    // -----------------------------------------------------------------------------
    // Card coordinates and size by index
    // -----------------------------------------------------------------------------
    bool MissionPlanner::cardRect(size_t index, glm::vec2 &pos, glm::vec2 &size) const
    {
        size_t limit = maxVisibleCards();
        if (limit == 0 || index >= limit)
        {
            return false;
        }
        float spacing = 12.0f;
        float yStart = storyboardOrigin_.y + 90.0f;
        float yEnd = buttonPos_.y - 24.0f;
        pos = glm::vec2(storyboardOrigin_.x + 16.0f, yStart + index * (storyboardCardHeight_ + spacing));
        size = glm::vec2(storyboardSize_.x - 32.0f, storyboardCardHeight_);
        if (pos.y + size.y > yEnd)
        {
            return false;
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // Cursor position detection
    // -----------------------------------------------------------------------------
    bool MissionPlanner::cursorInsideMap() const
    {
        return (cursorPos_.x >= mapOrigin_.x && cursorPos_.x <= (mapOrigin_.x + mapSize_.x) &&
                cursorPos_.y >= mapOrigin_.y && cursorPos_.y <= (mapOrigin_.y + mapSize_.y));
    }

    bool MissionPlanner::cursorInsideProfile() const
    {
        return (cursorPos_.x >= profileOrigin_.x && cursorPos_.x <= (profileOrigin_.x + profileSize_.x) &&
                cursorPos_.y >= profileOrigin_.y && cursorPos_.y <= (profileOrigin_.y + profileSize_.y));
    }

    bool MissionPlanner::cursorInsideStoryboard() const
    {
        return (cursorPos_.x >= storyboardOrigin_.x && cursorPos_.x <= (storyboardOrigin_.x + storyboardSize_.x) &&
                cursorPos_.y >= storyboardOrigin_.y && cursorPos_.y <= (storyboardOrigin_.y + storyboardSize_.y));
    }

    // -----------------------------------------------------------------------------
    // Total mission length
    // -----------------------------------------------------------------------------
    float MissionPlanner::computeMissionLength() const
    {
        float length = 0.0f;
        glm::vec2 prev(workingMission_.startPosition.x, workingMission_.startPosition.z);
        for (const auto &wp : workingMission_.waypoints)
        {
            glm::vec2 cur(wp.position.x, wp.position.z);
            length += glm::length(cur - prev);
            prev = cur;
        }
        return length;
    }

    // -----------------------------------------------------------------------------
    // Additional methods (stubs or simple implementations)
    // -----------------------------------------------------------------------------
    void MissionPlanner::generateMissionReport()
    {
        std::cout << "===== MISSION REPORT =====" << std::endl;
        std::cout << "Name: " << workingMission_.name << std::endl;
        std::cout << "Description: " << workingMission_.description << std::endl;
        std::cout << "Waypoints: " << workingMission_.waypoints.size() << std::endl;
        for (size_t i = 0; i < workingMission_.waypoints.size(); ++i)
        {
            const auto &wp = workingMission_.waypoints[i];
            std::cout << i + 1 << ". " << wp.name << "  Pos(" << wp.position.x << ", " << wp.position.y << ", " << wp.position.z << ")" << std::endl;
        }
        std::cout << "Total length: " << computeMissionLength() / 1000.0f << " km" << std::endl;
        std::cout << "==========================" << std::endl;
    }

    void MissionPlanner::autoGenerateWaypoints(int pattern)
    {
        workingMission_.waypoints.clear();
        float radius = mapHalfExtent_ * 0.6f;
        switch (pattern)
        {
        case 0:
        {
            // Square pattern around center
            glm::vec2 c(mapCenter_);
            float alt = defaultAltitude_;
            workingMission_.waypoints.push_back({glm::vec3(c.x - radius, alt, c.y - radius), "NW"});
            workingMission_.waypoints.push_back({glm::vec3(c.x + radius, alt, c.y - radius), "NE"});
            workingMission_.waypoints.push_back({glm::vec3(c.x + radius, alt, c.y + radius), "SE"});
            workingMission_.waypoints.push_back({glm::vec3(c.x - radius, alt, c.y + radius), "SW"});
            break;
        }
        case 1:
        {
            // Circular pattern (6 points)
            glm::vec2 c(mapCenter_);
            float alt = defaultAltitude_;
            int pts = 6;
            for (int i = 0; i < pts; ++i)
            {
                float angle = glm::two_pi<float>() * (static_cast<float>(i) / pts);
                float x = c.x + std::cos(angle) * radius;
                float z = c.y + std::sin(angle) * radius;
                workingMission_.waypoints.push_back({glm::vec3(x, alt, z), "P" + std::to_string(i + 1)});
            }
            break;
        }
        default:
        {
            // Simple straight line north and back
            glm::vec2 c(mapCenter_);
            float alt = defaultAltitude_;
            workingMission_.waypoints.push_back({glm::vec3(c.x, alt, c.y - radius), "NORTH"});
            workingMission_.waypoints.push_back({glm::vec3(c.x + radius * 0.5f, alt, c.y), "EAST"});
            workingMission_.waypoints.push_back({glm::vec3(c.x, alt, c.y + radius), "SOUTH"});
            workingMission_.waypoints.push_back({glm::vec3(c.x - radius * 0.5f, alt, c.y), "WEST"});
            break;
        }
        }
        selectedIndex_ = -1;
        hoveredCardIndex_ = -1;
        cachedMissionLength_ = computeMissionLength();
    }

    bool MissionPlanner::validateMission() const
    {
        return !workingMission_.waypoints.empty();
    }

    void MissionPlanner::saveMissionToFile(const std::string &path)
    {
        std::ofstream out(path);
        if (!out.is_open())
        {
            std::cerr << "Could not open file for writing: " << path << std::endl;
            return;
        }
        out << "{\n";
        out << "  \"id\": \"" << workingMission_.id << "\",\n";
        out << "  \"name\": \"" << workingMission_.name << "\",\n";
        out << "  \"description\": \"" << workingMission_.description << "\",\n";
        out << "  \"category\": \"" << workingMission_.category << "\",\n";
        out << "  \"difficulty\": " << workingMission_.difficulty << ",\n";
        out << "  \"environment\": {\n";
        out << "    \"timeOfDay\": \"" << workingMission_.environment.timeOfDay << "\",\n";
        out << "    \"weather\": \"" << workingMission_.environment.weather << "\",\n";
        out << "    \"windSpeed\": " << workingMission_.environment.windSpeed << ",\n";
        out << "    \"windDirection\": " << workingMission_.environment.windDirection << "\n";
        out << "  },\n";
        out << "  \"startPosition\": { \"x\": " << workingMission_.startPosition.x << ", \"y\": " << workingMission_.startPosition.y << ", \"z\": " << workingMission_.startPosition.z << " },\n";
        out << "  \"waypoints\": [\n";
        for (size_t i = 0; i < workingMission_.waypoints.size(); ++i)
        {
            const auto &wp = workingMission_.waypoints[i];
            out << "    { \"name\": \"" << wp.name << "\", \"position\": { \"x\": " << wp.position.x << ", \"y\": " << wp.position.y << ", \"z\": " << wp.position.z << " } }";
            if (i + 1 < workingMission_.waypoints.size())
            {
                out << ",";
            }
            out << "\n";
        }
        out << "  ]\n";
        out << "}\n";
        out.close();
    }

    void MissionPlanner::loadMissionFromFile(const std::string &path)
    {
        // Simple load: reading entire file and delegating to MissionRegistry is not available here
        std::ifstream in(path);
        if (!in.is_open())
        {
            std::cerr << "Could not open file for reading: " << path << std::endl;
            return;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        in.close();
        // Delegating to registry would be better, but rudimentary parsing for basic fields
        std::string content = buffer.str();
        auto extractString = [&](const std::string &key)
        {
            std::string search = "\"" + key + "\"";
            size_t kpos = content.find(search);
            if (kpos == std::string::npos)
                return std::string();
            size_t colon = content.find(':', kpos);
            size_t firstQuote = content.find('"', colon);
            size_t secondQuote = content.find('"', firstQuote + 1);
            if (firstQuote == std::string::npos || secondQuote == std::string::npos)
                return std::string();
            return content.substr(firstQuote + 1, secondQuote - firstQuote - 1);
        };
        auto extractFloat = [&](const std::string &key, float def)
        {
            std::string search = "\"" + key + "\"";
            size_t kpos = content.find(search);
            if (kpos == std::string::npos)
                return def;
            size_t colon = content.find(':', kpos);
            size_t valStart = colon + 1;
            while (valStart < content.size() && (content[valStart] == ' ' || content[valStart] == '\n' || content[valStart] == '\t'))
                ++valStart;
            size_t valEnd = valStart;
            while (valEnd < content.size() && content[valEnd] != ',' && content[valEnd] != '}' && content[valEnd] != ']')
                ++valEnd;
            std::string v = content.substr(valStart, valEnd - valStart);
            try
            {
                return std::stof(v);
            }
            catch (...)
            {
                return def;
            }
        };
        mission::MissionDefinition m;
        m.id = extractString("id");
        m.name = extractString("name");
        m.description = extractString("description");
        m.category = extractString("category");
        m.environment.timeOfDay = extractString("timeOfDay");
        m.environment.weather = extractString("weather");
        m.environment.windSpeed = extractFloat("windSpeed", 0.0f);
        m.environment.windDirection = extractFloat("windDirection", 0.0f);
        // Read startPosition
        m.startPosition.x = extractFloat("x", 0.0f);
        m.startPosition.y = extractFloat("y", 1500.0f);
        m.startPosition.z = extractFloat("z", 0.0f);
        // Waypoints are not parsed in this method
        loadMission(m);
    }

} // namespace ui
