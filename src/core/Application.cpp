#include "core/Application.h"

#include <iostream>
#include <stdexcept>

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include "gfx/skybox/TextureCube.h"
#include "gfx/skybox/SkyboxRenderer.h"
#include "gfx/terrain/TerrainPlane.h"
#include "gfx/core/Shader.h"
#include "gfx/geometry/Model.h"
#include "mission/MissionController.h"
#include "systems/FlightSimulationController.h"
#include "systems/CameraRig.h"
#include "systems/WaypointSystem.h"
#include "ui/UIManager.h"
#include "states/MenuState.h"
#include "states/PlanningState.h"
#include "states/FlightState.h"

namespace
{
    const char *kWindowTitle = "Flight Simulator HUD - F16 Model";
    constexpr int kInitialWidth = 1280;
    constexpr int kInitialHeight = 720;
    const char *kMissionPath = "assets/missions/missions.json";
}

namespace core
{

    Application::Application()
        : missionController_(std::make_unique<mission::MissionController>()),
          uiManager_(std::make_unique<ui::UIManager>()),
          flightController_(std::make_unique<systems::FlightSimulationController>()),
          cameraRig_(std::make_unique<systems::CameraRig>()),
          waypointSystem_(std::make_unique<systems::WaypointSystem>())
    {
        // Initialize the shared context with default values
        context_.appState = mission::AppState::Menu;
        context_.screenWidth = kInitialWidth;
        context_.screenHeight = kInitialHeight;
    }

    Application::~Application()
    {
        shutdown();
    }

    int Application::run()
    {
        // Attempt to initialize the application
        if (!initialize())
        {
            return EXIT_FAILURE;
        }

        // Enter the main game loop
        mainLoop();
        return EXIT_SUCCESS;
    }

    bool Application::initialize()
    {
        // 1. Initialize GLFW
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwInitialized_ = true;

        // 2. Create the window and context
        if (!initWindow())
        {
            return false;
        }

        // 3. Initialize GLAD (OpenGL function pointers)
        if (!initGLAD())
        {
            return false;
        }

        // 4. Load resources (models, textures, etc.)
        if (!initResources())
        {
            return false;
        }

        // 5. Initialize application states
        initStates();
        context_.lastFrame = static_cast<float>(glfwGetTime());
        return true;
    }

    bool Application::initWindow()
    {
        // Configure GLFW context version and profile (OpenGL 3.3 Core)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Create the window
        window_ = glfwCreateWindow(kInitialWidth, kInitialHeight, kWindowTitle, nullptr, nullptr);
        if (!window_)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            return false;
        }

        // Set context and callbacks
        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1); // Enable V-Sync
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *window, int width, int height)
                                       {
        if (auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))) {
            app->onWindowResized(width, height);
        } });

        context_.window = window_;

        // Get actual framebuffer size
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        if (width == 0 || height == 0)
        {
            width = kInitialWidth;
            height = kInitialHeight;
        }
        onWindowResized(width, height);

        return true;
    }

    bool Application::initGLAD()
    {
        // Load OpenGL function pointers
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        gladInitialized_ = true;

        // Log OpenGL info
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        glViewport(0, 0, context_.screenWidth, context_.screenHeight);
        return true;
    }

    bool Application::initResources()
    {
        // Load mission data
        if (!missionController_->loadMissions(kMissionPath))
        {
            return false;
        }

        // Initialize UI
        if (!uiManager_->initialize(context_.screenWidth, context_.screenHeight, &missionController_->registry()))
        {
            std::cerr << "Failed to initialize UI Manager" << std::endl;
            return false;
        }

        // Initialize simulation systems
        flightController_->initialize();
        cameraRig_->initialize(flightController_->planePosition(), flightController_->planeOrientation());
        waypointSystem_->initialize();

        // Configure terrain plane (flat, textured)
        terrainConfig_ = std::make_unique<gfx::TerrainConfig>();
        terrainConfig_->segments = 32;
        terrainConfig_->segmentSize = 2000.0f;
        terrainConfig_->textureTiling = 40.0f;
        terrainConfig_->fogMinDist = 1000.0f;
        terrainConfig_->fogMaxDist = 100000.0f;

        try
        {
            // Load skybox
            cubemap_ = std::make_unique<gfx::TextureCube>();
            if (!cubemap_->loadFromAtlas("assets/textures/skybox/Cubemap_Sky_22-512x512.png", false))
            {
                std::cerr << "Failed to load cubemap atlas" << std::endl;
                return false;
            }

            skybox_ = std::make_unique<gfx::SkyboxRenderer>();
            skybox_->init();
            skybox_->setCubemap(cubemap_.get());

            // Initialize flat terrain
            terrain_ = std::make_unique<gfx::TerrainPlane>(*terrainConfig_);
            terrain_->init();
            if (!terrain_->loadTexture("assets/textures/terrain"))
            {
                std::cerr << "Failed to load terrain textures" << std::endl;
                return false;
            }

            // Load aircraft model
            modelShader_ = std::make_unique<gfx::Shader>("shaders/model.vert", "shaders/model.frag");
            aircraftModel_ = std::make_unique<Model>("assets/models/f16.glb");
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Error initializing graphics resources: " << ex.what() << std::endl;
            return false;
        }

        // Populate context with resource pointers
        context_.missionController = missionController_.get();
        context_.uiManager = uiManager_.get();
        context_.flightController = flightController_.get();
        context_.cameraRig = cameraRig_.get();
        context_.waypointSystem = waypointSystem_.get();
        context_.terrainConfig = terrainConfig_.get();
        context_.terrain = terrain_.get();
        context_.skybox = skybox_.get();
        context_.cubemap = cubemap_.get();
        context_.modelShader = modelShader_.get();
        context_.aircraftModel = aircraftModel_.get();

        return true;
    }

    void Application::initStates()
    {
        // Register available states
        states_[mission::AppState::Menu] = std::make_unique<states::MenuState>();
        states_[mission::AppState::Planning] = std::make_unique<states::PlanningState>();
        states_[mission::AppState::Running] = std::make_unique<states::FlightState>();

        // Start in the Menu state
        activeStateId_ = mission::AppState::Menu;
        activeState_ = states_[activeStateId_].get();
        if (activeState_)
        {
            activeState_->onEnter(context_);
        }
    }

    void Application::mainLoop()
    {
        while (window_ && !glfwWindowShouldClose(window_))
        {
            updateTiming();
            handleResize();

            // Clear buffers
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Update and render the active state
            if (activeState_)
            {
                activeState_->handleInput(context_);
                activeState_->update(context_);
                activeState_->render(context_);
            }

            // Check for state transitions
            transitionIfNeeded();

            glfwSwapBuffers(window_);
            glfwPollEvents();
        }
    }

    void Application::updateTiming()
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        context_.deltaTime = currentFrame - context_.lastFrame;
        context_.lastFrame = currentFrame;
    }

    void Application::handleResize()
    {
        if (!window_)
        {
            return;
        }
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window_, &width, &height);
        if (width <= 0 || height <= 0)
        {
            return;
        }

        if (width != context_.screenWidth || height != context_.screenHeight)
        {
            onWindowResized(width, height);
        }
    }

    void Application::onWindowResized(int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return;
        }
        context_.screenWidth = width;
        context_.screenHeight = height;
        if (gladInitialized_)
        {
            glViewport(0, 0, width, height);
        }
        if (context_.uiManager)
        {
            context_.uiManager->resize(width, height);
        }
    }

    void Application::transitionIfNeeded()
    {
        if (!activeState_)
        {
            return;
        }

        const mission::AppState desired = context_.appState;
        if (desired == activeStateId_)
        {
            return;
        }

        if (desired == mission::AppState::Exiting)
        {
            if (window_)
            {
                glfwSetWindowShouldClose(window_, true);
            }
            return;
        }

        auto it = states_.find(desired);
        if (it == states_.end())
        {
            std::cerr << "Requested state not registered" << std::endl;
            return;
        }

        activeState_->onExit(context_);
        activeStateId_ = desired;
        activeState_ = it->second.get();
        activeState_->onEnter(context_);
    }

    void Application::shutdown()
    {
        if (shuttingDown_)
        {
            return;
        }
        shuttingDown_ = true;

        // Exit the current state
        if (activeState_)
        {
            activeState_->onExit(context_);
            activeState_ = nullptr;
        }
        states_.clear();

        // Release resources
        aircraftModel_.reset();
        modelShader_.reset();
        terrain_.reset();
        skybox_.reset();
        cubemap_.reset();
        terrainConfig_.reset();

        waypointSystem_.reset();
        cameraRig_.reset();
        flightController_.reset();
        uiManager_.reset();
        missionController_.reset();

        // Reset context
        context_ = AppContext{};

        // Destroy window and terminate GLFW
        if (window_)
        {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }

        if (glfwInitialized_)
        {
            glfwTerminate();
            glfwInitialized_ = false;
        }
    }

} // namespace core
