/**
 * @file MissionMenu.cpp
 * @brief Implementación del menú de misiones
 */

#include "MissionMenu.h"
#include <iostream>
#include <algorithm>

namespace ui
{

    MissionMenu::MissionMenu()
        : registry_(nullptr),
          screenWidth_(1280),
          screenHeight_(720),
          selectedIndex_(0),
          enterKeyPressed_(false),
          escKeyPressed_(false),
          upKeyPressed_(false),
          downKeyPressed_(false)
    {
    }

    void MissionMenu::init(mission::MissionRegistry *registry, int screenWidth, int screenHeight)
    {
        registry_ = registry;
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;
        selectedIndex_ = 0;

        renderer2D_.init(screenWidth, screenHeight);

        std::cout << "✓ MissionMenu initialized" << std::endl;
    }

    void MissionMenu::update(GLFWwindow *window, float deltaTime)
    {
        handleInput(window);
    }

    void MissionMenu::handleInput(GLFWwindow *window)
    {
        if (!registry_ || registry_->getMissionCount() == 0)
        {
            return;
        }

        // Navegación arriba/abajo
        bool upKeyCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
                                      glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
        bool downKeyCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
                                        glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);

        if (upKeyCurrentlyPressed && !upKeyPressed_)
        {
            selectedIndex_--;
            if (selectedIndex_ < 0)
            {
                selectedIndex_ = static_cast<int>(registry_->getMissionCount()) - 1;
            }
            upKeyPressed_ = true;
        }
        else if (!upKeyCurrentlyPressed)
        {
            upKeyPressed_ = false;
        }

        if (downKeyCurrentlyPressed && !downKeyPressed_)
        {
            selectedIndex_++;
            if (selectedIndex_ >= static_cast<int>(registry_->getMissionCount()))
            {
                selectedIndex_ = 0;
            }
            downKeyPressed_ = true;
        }
        else if (!downKeyCurrentlyPressed)
        {
            downKeyPressed_ = false;
        }

        // Selección (Enter o Espacio)
        bool enterCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS ||
                                      glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        if (enterCurrentlyPressed && !enterKeyPressed_)
        {
            result_.missionSelected = true;
            result_.selectedMissionIndex = selectedIndex_;
            enterKeyPressed_ = true;
            std::cout << "Misión seleccionada: " << selectedIndex_ << std::endl;
        }
        else if (!enterCurrentlyPressed)
        {
            enterKeyPressed_ = false;
        }

        // Salir (ESC)
        bool escCurrentlyPressed = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);

        if (escCurrentlyPressed && !escKeyPressed_)
        {
            result_.exitRequested = true;
            escKeyPressed_ = true;
        }
        else if (!escCurrentlyPressed)
        {
            escKeyPressed_ = false;
        }
    }

    void MissionMenu::render()
    {
        renderer2D_.begin();

        renderBackground();
        renderTitle();
        renderMissionList();
        renderMissionDetails();
        renderInstructions();

        renderer2D_.end();
        // flush() ya es llamado por end() internamente
    }

    void MissionMenu::renderBackground()
    {
        // Fondo degradado oscuro
        glm::vec4 bgColor(0.05f, 0.08f, 0.12f, 1.0f);
        renderer2D_.drawRect(glm::vec2(0, 0), glm::vec2(screenWidth_, screenHeight_), bgColor, true);

        // Líneas decorativas
        glm::vec4 accentColor(0.2f, 0.5f, 0.8f, 0.3f);
        renderer2D_.drawLine(glm::vec2(0, screenHeight_ * 0.15f),
                             glm::vec2(screenWidth_, screenHeight_ * 0.15f),
                             accentColor, 2.0f);
        renderer2D_.drawLine(glm::vec2(0, screenHeight_ * 0.85f),
                             glm::vec2(screenWidth_, screenHeight_ * 0.85f),
                             accentColor, 2.0f);
    }

    void MissionMenu::renderTitle()
    {
        // Barra superior con título
        float barHeight = screenHeight_ * 0.12f;
        renderer2D_.drawRect(glm::vec2(0, 0),
                             glm::vec2(screenWidth_, barHeight),
                             glm::vec4(0.05f, 0.1f, 0.15f, 0.95f), true);

        // Línea de acento
        renderer2D_.drawRect(glm::vec2(0, barHeight - 3),
                             glm::vec2(screenWidth_, 3),
                             glm::vec4(0.2f, 0.6f, 1.0f, 1.0f), true);

        // Título simplificado - solo texto clave
        glm::vec2 titlePos(screenWidth_ * 0.5f, barHeight * 0.35f);
        gfx::TextRenderer::drawString(renderer2D_, "F16 SIMULATOR",
                                      titlePos, glm::vec2(22, 40),
                                      glm::vec4(0.3f, 0.8f, 1.0f, 1.0f), 28.0f);
    }

    void MissionMenu::renderMissionList()
    {
        if (!registry_ || registry_->getMissionCount() == 0)
        {
            return;
        }

        // Panel de lista de misiones - más grande y centrado
        float listX = screenWidth_ * 0.15f;
        float listY = screenHeight_ * 0.18f;
        float listWidth = screenWidth_ * 0.70f;
        float itemHeight = 90.0f;
        float panelHeight = screenHeight_ * 0.65f;

        // Fondo del panel con borde
        renderer2D_.drawRect(glm::vec2(listX, listY),
                             glm::vec2(listWidth, panelHeight),
                             glm::vec4(0.06f, 0.09f, 0.14f, 0.95f), true);
        renderer2D_.drawRect(glm::vec2(listX, listY),
                             glm::vec2(listWidth, panelHeight),
                             glm::vec4(0.15f, 0.4f, 0.7f, 0.5f), false);

        // Renderizar cada misión
        float itemY = listY + 15.0f;
        size_t missionCount = registry_->getMissionCount();

        for (size_t i = 0; i < missionCount; ++i)
        {
            const mission::MissionDefinition *mission = registry_->getMissionByIndex(i);
            if (!mission)
                continue;

            bool isSelected = (static_cast<int>(i) == selectedIndex_);

            // Contenedor del item
            float itemX = listX + 15.0f;
            float itemW = listWidth - 30.0f;

            // Fondo del item con gradiente visual
            if (isSelected)
            {
                // Item seleccionado - destacado
                renderer2D_.drawRect(glm::vec2(itemX, itemY),
                                     glm::vec2(itemW, itemHeight - 5),
                                     glm::vec4(0.15f, 0.45f, 0.75f, 0.9f), true);
                // Borde brillante
                renderer2D_.drawRect(glm::vec2(itemX, itemY),
                                     glm::vec2(itemW, itemHeight - 5),
                                     glm::vec4(0.3f, 0.8f, 1.0f, 1.0f), false);
                // Barra lateral de selección
                renderer2D_.drawRect(glm::vec2(itemX, itemY),
                                     glm::vec2(6, itemHeight - 5),
                                     glm::vec4(0.2f, 0.9f, 0.4f, 1.0f), true);
            }
            else
            {
                renderer2D_.drawRect(glm::vec2(itemX, itemY),
                                     glm::vec2(itemW, itemHeight - 5),
                                     glm::vec4(0.08f, 0.12f, 0.18f, 0.7f), true);
            }

            // Número de misión (grande y visible)
            std::string numStr = std::to_string(i + 1);
            glm::vec4 numColor = isSelected ? glm::vec4(1.0f, 1.0f, 0.3f, 1.0f) : glm::vec4(0.4f, 0.6f, 0.8f, 1.0f);
            gfx::TextRenderer::drawString(renderer2D_, numStr,
                                          glm::vec2(itemX + 30, itemY + 25),
                                          glm::vec2(16, 28), numColor, 22.0f);

            // Nombre de la misión (más legible)
            glm::vec4 textColor = isSelected ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.75f, 0.8f, 0.9f, 1.0f);

            // Simplificar nombre si es muy largo
            std::string displayName = mission->name;
            if (displayName.length() > 25)
            {
                displayName = displayName.substr(0, 22) + "...";
            }

            gfx::TextRenderer::drawString(renderer2D_, displayName,
                                          glm::vec2(itemX + 80, itemY + 18),
                                          glm::vec2(11, 18), textColor, 14.0f);

            // Indicadores visuales de dificultad (estrellas como barras)
            float starX = itemX + 80;
            float starY = itemY + 48;
            for (int d = 0; d < 5; ++d)
            {
                glm::vec4 starColor = (d < mission->difficulty) ? glm::vec4(1.0f, 0.7f, 0.0f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.3f, 0.5f);
                renderer2D_.drawRect(glm::vec2(starX + d * 18, starY),
                                     glm::vec2(14, 8),
                                     starColor, true);
            }

            // Waypoints count (visual)
            float wpX = itemX + itemW - 100;
            gfx::TextRenderer::drawString(renderer2D_,
                                          std::to_string(mission->waypoints.size()),
                                          glm::vec2(wpX, itemY + 30),
                                          glm::vec2(14, 24),
                                          glm::vec4(0.3f, 0.9f, 0.5f, 1.0f), 18.0f);

            // Label "WP"
            gfx::TextRenderer::drawString(renderer2D_, "WP",
                                          glm::vec2(wpX + 40, itemY + 30),
                                          glm::vec2(8, 14),
                                          glm::vec4(0.5f, 0.7f, 0.5f, 0.8f), 10.0f);

            itemY += itemHeight;

            // Limitar cantidad visible
            if (itemY > listY + panelHeight - 20)
                break;
        }
    }

    void MissionMenu::renderMissionDetails()
    {
        // Panel de detalles movido abajo como banner
        if (!registry_ || selectedIndex_ < 0 ||
            selectedIndex_ >= static_cast<int>(registry_->getMissionCount()))
        {
            return;
        }

        const mission::MissionDefinition *mission = registry_->getMissionByIndex(selectedIndex_);
        if (!mission)
            return;

        // Banner inferior con información de la misión seleccionada
        float bannerHeight = 50.0f;
        float bannerY = screenHeight_ * 0.85f;

        renderer2D_.drawRect(glm::vec2(0, bannerY),
                             glm::vec2(screenWidth_, bannerHeight),
                             glm::vec4(0.08f, 0.15f, 0.22f, 0.95f), true);

        // Línea superior del banner
        renderer2D_.drawRect(glm::vec2(0, bannerY),
                             glm::vec2(screenWidth_, 2),
                             glm::vec4(0.2f, 0.6f, 1.0f, 0.8f), true);

        // Botón ENTER - destacado
        float btnW = 180.0f;
        float btnH = 35.0f;
        float btnX = screenWidth_ - btnW - 30;
        float btnY = bannerY + (bannerHeight - btnH) * 0.5f;

        renderer2D_.drawRect(glm::vec2(btnX, btnY),
                             glm::vec2(btnW, btnH),
                             glm::vec4(0.15f, 0.7f, 0.3f, 0.95f), true);
        renderer2D_.drawRect(glm::vec2(btnX, btnY),
                             glm::vec2(btnW, btnH),
                             glm::vec4(0.3f, 1.0f, 0.5f, 1.0f), false);

        gfx::TextRenderer::drawString(renderer2D_, "ENTER",
                                      glm::vec2(btnX + btnW * 0.5f, btnY + btnH * 0.35f),
                                      glm::vec2(14, 22),
                                      glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 18.0f);
    }

    void MissionMenu::renderInstructions()
    {
        // Instrucciones en la parte inferior
        float instrY = screenHeight_ * 0.935f;

        // Fondo semi-transparente
        renderer2D_.drawRect(glm::vec2(0, instrY - 5),
                             glm::vec2(screenWidth_, 50),
                             glm::vec4(0.02f, 0.04f, 0.06f, 0.9f), true);

        // Instrucciones simplificadas con indicadores visuales
        float xPos = screenWidth_ * 0.5f;
        gfx::TextRenderer::drawString(renderer2D_, "W/S SELECT    ENTER START    ESC EXIT",
                                      glm::vec2(xPos, instrY),
                                      glm::vec2(9, 15),
                                      glm::vec4(0.5f, 0.6f, 0.7f, 1.0f), 11.0f);
    }

    MenuResult MissionMenu::getResult() const
    {
        return result_;
    }

    void MissionMenu::reset()
    {
        result_.missionSelected = false;
        result_.exitRequested = false;
        result_.selectedMissionIndex = -1;
        enterKeyPressed_ = false;
        escKeyPressed_ = false;
    }

    void MissionMenu::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        renderer2D_.setScreenSize(width, height);
    }

    void MissionMenu::preselectMission(int index)
    {
        if (registry_ && index >= 0 && index < static_cast<int>(registry_->getMissionCount()))
        {
            selectedIndex_ = index;
        }
    }

} // namespace ui
