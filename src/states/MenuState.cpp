#include "states/MenuState.h"

#include <iostream>

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include "core/AppContext.h"
#include "mission/MissionController.h"
#include "mission/MissionRegistry.h"
#include "ui/UIManager.h"

namespace states
{

    void MenuState::onEnter(core::AppContext &context)
    {
        // Entering menu: set state and restore last selected mission if persisted.
        context.appState = mission::AppState::Menu;
        if (!menuStateRestored_ && context.missionController)
        {
            mission::MenuState savedState = context.missionController->loadMenuState();
            context.uiManager->preselectMission(savedState.lastMissionIndex);
            menuStateRestored_ = true;
        }
        if (context.uiManager)
        {
            context.uiManager->resetMenu();
        }
    }

    void MenuState::handleInput(core::AppContext &context)
    {
        // Pure 2D UI: depth not needed while navigating menu.
        glDisable(GL_DEPTH_TEST);
        if (context.uiManager)
        {
            context.uiManager->updateMenu(context.window, context.deltaTime);
        }
    }

    void MenuState::update(core::AppContext &context)
    {
        if (!context.uiManager || !context.missionController)
        {
            return;
        }

        ui::MenuResult result = context.uiManager->getMenuResult();
        if (result.exitRequested)
        {
            // ESC in menu requests closing the app.
            glfwSetWindowShouldClose(context.window, true);
            return;
        }

        if (result.missionSelected)
        {
            // Mission picked: persist selection, preload planner, and move to Planning state.
            const mission::MissionDefinition *selectedMission =
                context.missionController->registry().getMissionByIndex(result.selectedMissionIndex);
            if (selectedMission)
            {
                context.missionController->setCurrentMission(*selectedMission);
                mission::MenuState saveState;
                saveState.lastMissionId = selectedMission->id;
                saveState.lastMissionIndex = result.selectedMissionIndex;
                context.missionController->saveMenuState(saveState);

                context.uiManager->resetMenu();
                context.uiManager->loadPlannerMission(*selectedMission);
                context.uiManager->resetPlanner();

                context.appState = mission::AppState::Planning;
            }
        }
    }

    void MenuState::render(core::AppContext &context)
    {
        if (context.uiManager)
        {
            context.uiManager->renderMenu();
        }
    }

} // namespace states
