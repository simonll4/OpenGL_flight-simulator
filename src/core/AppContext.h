/**
 * @file AppContext.h
 * @brief Shared application context containing global state and system references.
 */

#pragma once

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include "mission/AppState.h"

namespace mission
{
    class MissionController;
    class MissionRuntime;
}

namespace ui
{
    class UIManager;
}

namespace systems
{
    class FlightSimulationController;
    class CameraRig;
    class WaypointSystem;
}

namespace gfx
{
    struct TerrainConfig;
    class TerrainPlane;
    class SkyboxRenderer;
    class TextureCube;
    class Shader;
}

class Model;

namespace core
{

    /**
     * @brief Shared application context containing global state and system references.
     *
     * This struct serves as a central hub for accessing core systems, current application state,
     * and frame timing information. It is passed around to various states and systems to allow
     * them to interact with each other and the main application environment.
     */
    struct AppContext
    {
        // --- Core Application State ---
        GLFWwindow *window = nullptr;                         ///< Pointer to the main GLFW window.
        mission::AppState appState = mission::AppState::Menu; ///< Current state of the application (e.g., Menu, Flight, Planning).
        float deltaTime = 0.0f;                               ///< Time elapsed since the last frame (in seconds).
        float lastFrame = 0.0f;                               ///< Timestamp of the last frame (in seconds).
        int screenWidth = 1280;                               ///< Current width of the window/screen.
        int screenHeight = 720;                               ///< Current height of the window/screen.

        // --- Graphics Resources ---
        gfx::TerrainConfig *terrainConfig = nullptr; ///< Configuration for the flat terrain plane.
        gfx::TerrainPlane *terrain = nullptr;        ///< Pointer to the terrain renderer.
        gfx::SkyboxRenderer *skybox = nullptr;       ///< Pointer to the skybox renderer.
        gfx::TextureCube *cubemap = nullptr;         ///< Pointer to the skybox cubemap texture.
        gfx::Shader *modelShader = nullptr;          ///< Shader used for rendering models (e.g., aircraft).
        Model *aircraftModel = nullptr;              ///< Pointer to the 3D model of the aircraft.

        // --- Simulation Systems ---
        systems::FlightSimulationController *flightController = nullptr; ///< Controller for flight physics and dynamics.
        systems::CameraRig *cameraRig = nullptr;                         ///< System managing camera positioning and modes.
        systems::WaypointSystem *waypointSystem = nullptr;               ///< System for managing mission waypoints.

        // --- Game Logic & UI ---
        mission::MissionController *missionController = nullptr; ///< Controller for mission logic and state.
        ui::UIManager *uiManager = nullptr;                      ///< Manager for user interface elements.
    };

} // namespace core
