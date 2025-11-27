#include "states/FlightState.h"

#include <iostream>

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "core/AppContext.h"
#include "mission/MissionController.h"
#include "mission/MissionRuntime.h"
#include "systems/FlightSimulationController.h"
#include "systems/CameraRig.h"
#include "systems/WaypointSystem.h"
#include "ui/UIManager.h"
#include "gfx/skybox/SkyboxRenderer.h"
#include "gfx/terrain/TerrainPlane.h"
#include "gfx/core/Shader.h"
#include "gfx/geometry/Model.h"

namespace states
{

    void FlightState::onEnter(core::AppContext &context)
    {
        context.appState = mission::AppState::Running;
        completionPromptShown_ = false;
    }

    void FlightState::handleInput(core::AppContext &context)
    {
        if (!context.missionController || !context.uiManager)
        {
            return;
        }

        mission::MissionRuntime &runtime = context.missionController->runtime();

        // Let the overlay consume input first (briefing/completion screens).
        if (context.uiManager->handleOverlayInput(context.window))
        {
            if (context.uiManager->overlayReadyToFly())
            {
                runtime.confirmReadyToFly();
                context.uiManager->hideOverlay();
                std::cout << "[FlightState] Pilot ready to fly" << std::endl;
            }

            ui::CompletionChoice choice = context.uiManager->overlayChoice();
            if (choice == ui::CompletionChoice::ReturnToMenu)
            {
                runtime.requestMenuExit();
                std::cout << "[FlightState] Return to menu requested" << std::endl;
            }
            else if (choice == ui::CompletionChoice::FreeFlight)
            {
                runtime.continueFreeFlight();
                context.uiManager->hideOverlay();
                std::cout << "[FlightState] Continuing in free flight mode" << std::endl;
            }
        }

        // If overlay is visible, only allow ESC to exit to menu after completion.
        if (context.uiManager->isOverlayVisible())
        {
            if (glfwGetKey(context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                if (!overlayEscPressed_ && runtime.isCompleted())
                {
                    runtime.requestMenuExit();
                    overlayEscPressed_ = true;
                }
            }
            else
            {
                overlayEscPressed_ = false;
            }
            return;
        }

        // ESC closes the app.
        if (glfwGetKey(context.window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            if (!escPressed_)
            {
                glfwSetWindowShouldClose(context.window, true);
                escPressed_ = true;
                return;
            }
        }
        else
        {
            escPressed_ = false;
        }

        // TAB: request return to mission menu.
        if (glfwGetKey(context.window, GLFW_KEY_TAB) == GLFW_PRESS)
        {
            if (!tabPressed_)
            {
                runtime.requestMenuExit();
                std::cout << "\n>>> TAB pressed - Returning to mission menu...\n"
                          << std::endl;
                tabPressed_ = true;
            }
        }
        else
        {
            tabPressed_ = false;
        }

        // M: skip active waypoint.
        if (glfwGetKey(context.window, GLFW_KEY_M) == GLFW_PRESS)
        {
            if (!mPressed_ && context.waypointSystem)
            {
                context.waypointSystem->skipActiveWaypoint(runtime);
                mPressed_ = true;
            }
        }
        else
        {
            mPressed_ = false;
        }

        // R: restart current mission.
        if (glfwGetKey(context.window, GLFW_KEY_R) == GLFW_PRESS)
        {
            if (!rPressed_)
            {
                restartMission(context);
                rPressed_ = true;
            }
        }
        else
        {
            rPressed_ = false;
        }

        // Forward flight and camera controls to their systems.
        if (context.flightController)
        {
            context.flightController->handleControls(context.window, context.deltaTime);
        }
        if (context.cameraRig)
        {
            context.cameraRig->handleInput(context.window, context.deltaTime);
        }
    }

    void FlightState::update(core::AppContext &context)
    {
        if (!context.missionController || !context.flightController || !context.uiManager)
        {
            return;
        }

        mission::MissionRuntime &runtime = context.missionController->runtime();
        context.uiManager->updateOverlay(context.deltaTime);

        // If mission ended and prompt not shown yet, show completion overlay.
        if (runtime.isCompleted() && !completionPromptShown_)
        {
            context.uiManager->showCompletionPrompt(runtime);
            std::cout << "[FlightState] Waiting for pilot decision (SPACE=Free Flight, TAB=Menu)" << std::endl;
            completionPromptShown_ = true;
        }

        if (!runtime.isCompleted())
        {
            completionPromptShown_ = false;
        }

        // Handle menu exit request (overlay/TAB).
        if (runtime.menuExitRequested())
        {
            runtime.reset();
            context.uiManager->resetOverlay();
            if (context.waypointSystem)
            {
                context.waypointSystem->reset();
            }
            context.appState = mission::AppState::Menu;
            return;
        }

        // Physics runs only if runtime allows (not paused).
        if (runtime.shouldRunPhysics())
        {
            context.flightController->step(context.deltaTime);
        }

        // Update mission progress and metrics using latest flight data.
        runtime.updateProgress(context.flightController->getFlightData(), context.deltaTime);
        runtime.updateMetrics(context.flightController->getFlightData(), context.deltaTime);

        // Waypoints advance based on current plane position.
        if (context.waypointSystem)
        {
            context.waypointSystem->update(context.flightController->planePosition(),
                                           context.flightController->getFlightData(),
                                           runtime);
        }

        // Camera follows aircraft with its rig.
        if (context.cameraRig)
        {
            context.cameraRig->update(context.deltaTime,
                                      context.flightController->planePosition(),
                                      context.flightController->planeOrientation(),
                                      context.flightController->planeSpeed());
        }
    }

    void FlightState::render(core::AppContext &context)
    {
        if (!context.cameraRig || !context.flightController)
        {
            return;
        }

        glEnable(GL_DEPTH_TEST);

        const glm::mat4 &view = context.cameraRig->viewMatrix();
        glm::mat4 projection = context.cameraRig->projectionMatrix(context.screenWidth, context.screenHeight);

        // Background: skybox.
        if (context.skybox)
        {
            context.skybox->draw(view, projection);
        }

        // Flat terrain with fog tuned to camera altitude.
        if (context.terrainConfig && context.terrain)
        {
            context.terrainConfig->fogMinDist = context.cameraRig->position().y * 0.5f;
            context.terrainConfig->fogMaxDist = context.cameraRig->dynamicFarPlane() * 0.8f;
            context.terrain->draw(view, projection, context.cameraRig->position(), glm::vec3(0.5f, 0.7f, 1.0f));
        }

        // Aircraft model rendering with simple lighting.
        if (context.modelShader && context.aircraftModel)
        {
            context.modelShader->use();
            context.modelShader->setMat4("projection", projection);
            context.modelShader->setMat4("view", view);
            context.modelShader->setVec3("viewPos", context.cameraRig->position());

            glm::vec3 sunDirection = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
            glm::vec3 lightPosition = context.flightController->planePosition() + sunDirection * 500.0f;
            context.modelShader->setVec3("lightPos", lightPosition);
            context.modelShader->setVec3("lightColor", glm::vec3(1.5f, 1.5f, 1.4f));

            glm::vec3 modelForward = glm::vec3(+1, 0, 0);
            glm::vec3 modelUp = glm::vec3(0, 0, -1);
            glm::vec3 worldForward = glm::vec3(0, 0, -1);
            glm::vec3 worldUp = glm::vec3(0, +1, 0);

            glm::quat q1 = glm::rotation(glm::normalize(modelForward), glm::normalize(worldForward));
            glm::vec3 upAfter = glm::normalize(q1 * modelUp);
            glm::vec3 axis = glm::normalize(worldForward);
            glm::vec3 upProjected = glm::normalize(upAfter - glm::dot(upAfter, axis) * axis);
            float cosang = glm::clamp(glm::dot(upProjected, worldUp), -1.0f, 1.0f);
            float angle = acosf(cosang);
            glm::vec3 crossv = glm::cross(upProjected, worldUp);
            float sign = (glm::dot(crossv, axis) < 0.f) ? -1.f : +1.f;
            glm::quat q2 = glm::angleAxis(sign * angle, axis);
            glm::mat4 Rcorr = glm::mat4_cast(q2 * q1);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, context.flightController->planePosition());
            model = model * glm::mat4_cast(context.flightController->planeOrientation());
            model = model * Rcorr;
            model = glm::scale(model, glm::vec3(0.05f));
            context.modelShader->setMat4("model", model);

            context.aircraftModel->Draw(*context.modelShader);
        }

        // 3D waypoints.
        if (context.waypointSystem)
        {
            context.waypointSystem->render(view, projection, context.missionController->runtime());
        }

        // HUD only in first-person camera.
        if (context.cameraRig->isFirstPerson() && context.uiManager)
        {
            glDisable(GL_DEPTH_TEST);
            context.uiManager->updateHUD(context.flightController->getFlightData());
            context.uiManager->renderHUD();
            glEnable(GL_DEPTH_TEST);
        }

        // Briefing/completion overlay when needed.
        if (context.missionController->runtime().shouldShowOverlay() && context.uiManager)
        {
            glDisable(GL_DEPTH_TEST);
            context.uiManager->renderOverlay();
            glEnable(GL_DEPTH_TEST);
        }
    }

    void FlightState::restartMission(core::AppContext &context)
    {
        if (!context.missionController || !context.flightController)
        {
            std::cout << "[WARN] No active mission to restart" << std::endl;
            return;
        }

        if (!context.missionController->hasActiveMission())
        {
            std::cout << "[WARN] No active mission to restart" << std::endl;
            return;
        }

        context.missionController->startMission();
        if (context.waypointSystem)
        {
            context.waypointSystem->loadFromMission(context.missionController->currentMission());
        }
        context.flightController->resetForMission();
        if (context.cameraRig)
        {
            context.cameraRig->reset(context.flightController->planePosition(),
                                     context.flightController->planeOrientation());
        }
        if (context.uiManager)
        {
            context.uiManager->resetOverlay();
            context.uiManager->showBriefing(context.missionController->currentMission());
        }
        std::cout << "Mission restarted!" << std::endl;
    }

} // namespace states
