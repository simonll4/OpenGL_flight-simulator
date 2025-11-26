#include "states/PlanningState.h"

#include <iostream>

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include "core/AppContext.h"
#include "mission/MissionController.h"
#include "systems/FlightSimulationController.h"
#include "systems/CameraRig.h"
#include "systems/WaypointSystem.h"
#include "ui/UIManager.h"

namespace states
{

    void PlanningState::onEnter(core::AppContext &context)
    {
        context.appState = mission::AppState::Planning;
        if (context.uiManager)
        {
            context.uiManager->resetPlanner();
        }
    }

    void PlanningState::handleInput(core::AppContext &context)
    {
        glDisable(GL_DEPTH_TEST);
        if (context.uiManager)
        {
            context.uiManager->updatePlanner(context.window, context.deltaTime);
        }
    }

    void PlanningState::update(core::AppContext &context)
    {
        if (!context.uiManager || !context.missionController)
        {
            return;
        }

        ui::PlannerResult result = context.uiManager->getPlannerResult();
        if (result.cancelRequested)
        {
            context.uiManager->resetPlanner();
            context.appState = mission::AppState::Menu;
            return;
        }

        if (result.startRequested)
        {
            mission::MissionDefinition mission = context.uiManager->getPlannerMission();
            context.missionController->setCurrentMission(mission);
            context.missionController->startMission();

            if (context.waypointSystem)
            {
                context.waypointSystem->loadFromMission(mission);
            }
            if (context.uiManager)
            {
                context.uiManager->resetOverlay();
                context.uiManager->showBriefing(mission);
                context.uiManager->resetPlanner();
            }
            if (context.flightController)
            {
                context.flightController->resetForMission();
            }
            if (context.cameraRig && context.flightController)
            {
                context.cameraRig->reset(context.flightController->planePosition(),
                                         context.flightController->planeOrientation());
            }

            std::cout << "\n==============================================\n";
            std::cout << "  Mission started: " << mission.name << "\n";
            std::cout << "==============================================\n"
                      << std::endl;

            context.appState = mission::AppState::Running;
        }
    }

    void PlanningState::render(core::AppContext &context)
    {
        if (context.uiManager)
        {
            context.uiManager->renderPlanner();
        }
    }

} // namespace states
