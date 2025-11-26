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
#include "gfx/terrain/ClipmapTerrain.h"
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
        if (!initialize())
        {
            return EXIT_FAILURE;
        }

        mainLoop();
        return EXIT_SUCCESS;
    }

    bool Application::initialize()
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            return false;
        }
        glfwInitialized_ = true;

        if (!initWindow())
        {
            return false;
        }

        if (!initGLAD())
        {
            return false;
        }

        if (!initResources())
        {
            return false;
        }

        initStates();
        context_.lastFrame = static_cast<float>(glfwGetTime());
        return true;
    }

    bool Application::initWindow()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window_ = glfwCreateWindow(kInitialWidth, kInitialHeight, kWindowTitle, nullptr, nullptr);
        if (!window_)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            return false;
        }

        glfwMakeContextCurrent(window_);
        glfwSwapInterval(1);
        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *window, int width, int height)
                                       {
        if (auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window))) {
            app->onWindowResized(width, height);
        } });

        context_.window = window_;

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
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        gladInitialized_ = true;

        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

        glViewport(0, 0, context_.screenWidth, context_.screenHeight);
        return true;
    }

    bool Application::initResources()
    {
        if (!missionController_->loadMissions(kMissionPath))
        {
            return false;
        }

        if (!uiManager_->initialize(context_.screenWidth, context_.screenHeight, &missionController_->registry()))
        {
            std::cerr << "Failed to initialize UI Manager" << std::endl;
            return false;
        }

        flightController_->initialize();
        cameraRig_->initialize(flightController_->planePosition(), flightController_->planeOrientation());
        waypointSystem_->initialize();

        clipmapConfig_ = std::make_unique<gfx::ClipmapConfig>();
        clipmapConfig_->levels = 14;
        clipmapConfig_->segments = 64;
        clipmapConfig_->segmentSize = 4.0f;
        clipmapConfig_->heightScale = 3000.0f;
        clipmapConfig_->heightOffset = 0.0f;
        clipmapConfig_->terrainSize = 200000.0f;
        clipmapConfig_->fogMinDist = 1000.0f;
        clipmapConfig_->fogMaxDist = 100000.0f;

        try
        {
            cubemap_ = std::make_unique<gfx::TextureCube>();
            if (!cubemap_->loadFromAtlas("assets/textures/skybox/Cubemap_Sky_22-512x512.png", false))
            {
                std::cerr << "Failed to load cubemap atlas" << std::endl;
                return false;
            }

            skybox_ = std::make_unique<gfx::SkyboxRenderer>();
            skybox_->init();
            skybox_->setCubemap(cubemap_.get());

            terrain_ = std::make_unique<gfx::ClipmapTerrain>(*clipmapConfig_);
            terrain_->init();
            if (!terrain_->loadTextures("assets/textures/terrain"))
            {
                std::cerr << "Failed to load terrain textures" << std::endl;
                return false;
            }

            modelShader_ = std::make_unique<gfx::Shader>("shaders/model.vert", "shaders/model.frag");
            aircraftModel_ = std::make_unique<Model>("assets/models/f16.glb");
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Error initializing graphics resources: " << ex.what() << std::endl;
            return false;
        }

        context_.missionController = missionController_.get();
        context_.uiManager = uiManager_.get();
        context_.flightController = flightController_.get();
        context_.cameraRig = cameraRig_.get();
        context_.waypointSystem = waypointSystem_.get();
        context_.clipmapConfig = clipmapConfig_.get();
        context_.terrain = terrain_.get();
        context_.skybox = skybox_.get();
        context_.cubemap = cubemap_.get();
        context_.modelShader = modelShader_.get();
        context_.aircraftModel = aircraftModel_.get();

        return true;
    }

    void Application::initStates()
    {
        states_[mission::AppState::Menu] = std::make_unique<states::MenuState>();
        states_[mission::AppState::Planning] = std::make_unique<states::PlanningState>();
        states_[mission::AppState::Running] = std::make_unique<states::FlightState>();

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

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (activeState_)
            {
                activeState_->handleInput(context_);
                activeState_->update(context_);
                activeState_->render(context_);
            }

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

        if (activeState_)
        {
            activeState_->onExit(context_);
            activeState_ = nullptr;
        }
        states_.clear();

        aircraftModel_.reset();
        modelShader_.reset();
        terrain_.reset();
        skybox_.reset();
        cubemap_.reset();
        clipmapConfig_.reset();

        waypointSystem_.reset();
        cameraRig_.reset();
        flightController_.reset();
        uiManager_.reset();
        missionController_.reset();

        context_ = AppContext{};

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
