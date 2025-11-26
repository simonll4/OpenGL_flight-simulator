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
    struct ClipmapConfig;
    class ClipmapTerrain;
    class SkyboxRenderer;
    class TextureCube;
    class Shader;
}

class Model;

namespace core
{

    struct AppContext
    {
        GLFWwindow *window = nullptr;
        mission::AppState appState = mission::AppState::Menu;
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        int screenWidth = 1280;
        int screenHeight = 720;

        gfx::ClipmapConfig *clipmapConfig = nullptr;
        gfx::ClipmapTerrain *terrain = nullptr;
        gfx::SkyboxRenderer *skybox = nullptr;
        gfx::TextureCube *cubemap = nullptr;
        gfx::Shader *modelShader = nullptr;
        Model *aircraftModel = nullptr;

        systems::FlightSimulationController *flightController = nullptr;
        systems::CameraRig *cameraRig = nullptr;
        systems::WaypointSystem *waypointSystem = nullptr;

        mission::MissionController *missionController = nullptr;
        ui::UIManager *uiManager = nullptr;
    };

} // namespace core
