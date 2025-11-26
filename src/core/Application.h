/**
 * @file Application.h
 * @brief Main application class managing the flight simulator lifecycle.
 */

#pragma once

#include <memory>
#include <unordered_map>

#include "core/AppContext.h"

// ============================================================================
// Forward Declarations
// ============================================================================

/**
 * @namespace gfx
 * @brief Graphics subsystem for rendering terrain, skybox, models, and shaders.
 */
namespace gfx
{
    struct TerrainConfig; ///< Configuration parameters for the terrain plane.
    class TerrainPlane;   ///< Flat textured terrain renderer.
    class TextureCube;    ///< Cubemap texture wrapper for skybox rendering.
    class SkyboxRenderer; ///< Renderer for the environment skybox using cubemaps.
    class Shader;         ///< OpenGL shader program wrapper for vertex/fragment shaders.
}

class Model; ///< 3D model loader and renderer (e.g., aircraft mesh).

/**
 * @namespace mission
 * @brief Mission management system for loading, tracking, and controlling mission states.
 */
namespace mission
{
    enum class AppState;     ///< Application state enumeration (Menu, Planning, Flight, etc.).
    class MissionController; ///< Orchestrates mission lifecycle, objectives, and state transitions.
}

/**
 * @namespace states
 * @brief Application state machine for managing different operational modes.
 */
namespace states
{
    class IModeState;    ///< Interface for application state behaviors (update, render, input).
    class MenuState;     ///< Main menu state for mission selection and settings.
    class PlanningState; ///< Mission planning state for waypoint placement and route design.
    class FlightState;   ///< Active flight simulation state with physics and rendering.
}

/**
 * @namespace ui
 * @brief User interface system for menus, overlays, and HUD elements.
 */
namespace ui
{
    class UIManager; ///< Manages UI rendering, input handling, and screen transitions.
}

/**
 * @namespace systems
 * @brief Core gameplay systems for flight dynamics, camera control, and navigation.
 */
namespace systems
{
    class FlightSimulationController; ///< Manages aircraft physics, FDM integration, and flight dynamics.
    class CameraRig;                  ///< Camera system with multiple view modes (cockpit, chase, free).
    class WaypointSystem;             ///< Manages waypoint creation, navigation, and rendering.
}

namespace core
{

    /**
     * @brief Main application class managing the lifecycle of the flight simulator.
     *
     * The Application class is responsible for initializing the window, OpenGL context,
     * and all core subsystems (graphics, physics, UI, etc.). It runs the main game loop,
     * handles input, and manages transitions between different application states (Menu, Flight, etc.).
     */
    class Application
    {
    public:
        /**
         * @brief Constructs the Application instance.
         *
         * Sets up initial configuration and instantiates core system managers.
         */
        Application();

        /**
         * @brief Destructor.
         *
         * Ensures proper shutdown and cleanup of resources.
         */
        ~Application();

        /**
         * @brief Starts the application's main loop.
         *
         * This method initializes the application and enters the main loop until the window is closed
         * or a shutdown is requested.
         *
         * @return EXIT_SUCCESS on successful execution, EXIT_FAILURE otherwise.
         */
        int run();

    private:
        /**
         * @brief Initializes all application subsystems.
         *
         * @return true if initialization was successful, false otherwise.
         */
        bool initialize();

        /**
         * @brief Initializes the GLFW window and context.
         *
         * @return true if the window was created successfully, false otherwise.
         */
        bool initWindow();

        /**
         * @brief Initializes GLAD to load OpenGL function pointers.
         *
         * @return true if GLAD was initialized successfully, false otherwise.
         */
        bool initGLAD();

        /**
         * @brief Initializes game resources (models, textures, shaders, etc.).
         *
         * @return true if all resources were loaded successfully, false otherwise.
         */
        bool initResources();

        /**
         * @brief Initializes the application states (Menu, Flight, Planning).
         */
        void initStates();

        /**
         * @brief The main application loop.
         *
         * Handles timing, input, updating, and rendering for each frame.
         */
        void mainLoop();

        /**
         * @brief Updates the frame timing variables (deltaTime).
         */
        void updateTiming();

        /**
         * @brief Checks if the window has been resized and updates the context if necessary.
         */
        void handleResize();

        /**
         * @brief Callback for window resize events.
         *
         * @param width The new width of the window.
         * @param height The new height of the window.
         */
        void onWindowResized(int width, int height);

        /**
         * @brief Checks if a state transition is requested and performs it.
         */
        void transitionIfNeeded();

        /**
         * @brief Cleans up resources and shuts down the application.
         */
        void shutdown();

        // --- Core Window & Context ---
        GLFWwindow *window_ = nullptr; ///< The main GLFW window handle.
        AppContext context_{};         ///< The shared application context.

        // --- State Management ---
        std::unordered_map<mission::AppState, std::unique_ptr<states::IModeState>> states_; ///< Map of available application states.
        states::IModeState *activeState_ = nullptr;                                         ///< Pointer to the currently active state.
        mission::AppState activeStateId_ = mission::AppState::Menu;                         ///< ID of the currently active state.

        // --- Graphics Resources ---
        std::unique_ptr<gfx::TerrainConfig> terrainConfig_; ///< Configuration for the flat terrain plane.
        std::unique_ptr<gfx::TerrainPlane> terrain_;        ///< Flat terrain renderer.
        std::unique_ptr<gfx::TextureCube> cubemap_;         ///< Cubemap texture for skybox environment mapping.
        std::unique_ptr<gfx::SkyboxRenderer> skybox_;       ///< Skybox renderer for atmospheric background.
        std::unique_ptr<gfx::Shader> modelShader_;          ///< Shader program for rendering 3D models with lighting.
        std::unique_ptr<Model> aircraftModel_;              ///< 3D model of the aircraft mesh.

        // --- System Managers ---
        std::unique_ptr<mission::MissionController> missionController_;         ///< Manages mission objectives, progression, and completion.
        std::unique_ptr<ui::UIManager> uiManager_;                              ///< Handles UI rendering, menus, and HUD overlays.
        std::unique_ptr<systems::FlightSimulationController> flightController_; ///< Controls flight dynamics, physics integration, and FDM.
        std::unique_ptr<systems::CameraRig> cameraRig_;                         ///< Manages camera positioning and view modes.
        std::unique_ptr<systems::WaypointSystem> waypointSystem_;               ///< Handles waypoint management and navigation rendering.

        // --- Initialization Flags ---
        bool glfwInitialized_ = false; ///< Flag indicating if GLFW has been initialized.
        bool gladInitialized_ = false; ///< Flag indicating if GLAD has been initialized.
        bool shuttingDown_ = false;    ///< Flag indicating if the application is shutting down.
    };

} // namespace core
