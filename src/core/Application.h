#pragma once

#include <memory>
#include <unordered_map>

#include "core/AppContext.h"

namespace gfx {
struct ClipmapConfig;
class ClipmapTerrain;
class TextureCube;
class SkyboxRenderer;
class Shader;
}

class Model;

namespace mission {
enum class AppState;
class MissionController;
}

namespace states {
class IModeState;
class MenuState;
class PlanningState;
class FlightState;
}

namespace ui {
class UIManager;
}

namespace systems {
class FlightSimulationController;
class CameraRig;
class WaypointSystem;
}

namespace core {

class Application {
public:
    Application();
    ~Application();

    int run();

private:
    bool initialize();
    bool initWindow();
    bool initGLAD();
    bool initResources();
    void initStates();
    void mainLoop();
    void updateTiming();
    void handleResize();
    void onWindowResized(int width, int height);
    void transitionIfNeeded();
    void shutdown();

    GLFWwindow* window_ = nullptr;
    AppContext context_{};
    std::unordered_map<mission::AppState, std::unique_ptr<states::IModeState>> states_;
    states::IModeState* activeState_ = nullptr;
    mission::AppState activeStateId_ = mission::AppState::Menu;

    std::unique_ptr<gfx::ClipmapConfig> clipmapConfig_;
    std::unique_ptr<gfx::ClipmapTerrain> terrain_;
    std::unique_ptr<gfx::TextureCube> cubemap_;
    std::unique_ptr<gfx::SkyboxRenderer> skybox_;
    std::unique_ptr<gfx::Shader> modelShader_;
    std::unique_ptr<Model> aircraftModel_;

    std::unique_ptr<mission::MissionController> missionController_;
    std::unique_ptr<ui::UIManager> uiManager_;
    std::unique_ptr<systems::FlightSimulationController> flightController_;
    std::unique_ptr<systems::CameraRig> cameraRig_;
    std::unique_ptr<systems::WaypointSystem> waypointSystem_;

    bool glfwInitialized_ = false;
    bool gladInitialized_ = false;
    bool shuttingDown_ = false;
};

} // namespace core
