/**
 * @file MissionMenu.cpp
 * @brief Implementación del menú de misiones
 */

#include "MissionMenu.h"
#include <iostream>
#include <algorithm>

extern "C"
{
#include <glad/glad.h>
}

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

        const std::string fontPath = "assets/fonts/RobotoMono-Regular.ttf";
        // Usar tamaño base mayor (96px) y atlas más grande para mejor calidad
        menuFontReady_ = menuFont_.loadFromFile(fontPath, 96.0f, 2048);
        if (!menuFontReady_)
        {
            std::cerr << "[MissionMenu] No se pudo cargar la fuente RobotoMono en " << fontPath << std::endl;
        }

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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        renderer2D_.begin();

        renderBackground();
        renderTitle();
        renderMissionList();
        renderMissionDetails();
        renderInstructions();

        renderer2D_.end();
        glDisable(GL_BLEND);
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
        glm::vec2 titlePos(screenWidth_ * 0.5f, barHeight * 0.38f);
        drawMenuText("F16 FLIGHT SIM", titlePos, 34.0f,
                     glm::vec4(0.3f, 0.8f, 1.0f, 1.0f), glm::vec2(0.5f, 0.5f));
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
            drawMenuText(numStr,
                         glm::vec2(itemX + 32.0f, itemY + (itemHeight - 5) * 0.45f),
                         30.0f, numColor, glm::vec2(0.0f, 0.5f));

            // Nombre de la misión (más legible)
            glm::vec4 textColor = isSelected ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(0.75f, 0.8f, 0.9f, 1.0f);

            // Simplificar nombre si es muy largo
            std::string displayName = mission->name;
            if (displayName.length() > 25)
            {
                displayName = displayName.substr(0, 22) + "...";
            }

            drawMenuText(displayName,
                         glm::vec2(itemX + 80.0f, itemY + 24.0f),
                         20.0f, textColor, glm::vec2(0.0f, 0.5f));

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
            drawMenuText(std::to_string(mission->waypoints.size()),
                         glm::vec2(wpX, itemY + 32.0f),
                         22.0f, glm::vec4(0.3f, 0.9f, 0.5f, 1.0f), glm::vec2(0.0f, 0.5f));

            // Label "WP"
            drawMenuText("WP",
                         glm::vec2(wpX + 38.0f, itemY + 32.0f),
                         16.0f, glm::vec4(0.5f, 0.7f, 0.5f, 0.85f), glm::vec2(0.0f, 0.5f));

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

        drawMenuText("ENTER",
                     glm::vec2(btnX + btnW * 0.5f, btnY + btnH * 0.5f),
                     20.0f, glm::vec4(1.0f), glm::vec2(0.5f, 0.5f));
    }

    void MissionMenu::renderInstructions()
    {
        // Instrucciones en la parte inferior
        float instrY = screenHeight_ * 0.935f;

        // Fondo semi-transparente
        renderer2D_.drawRect(glm::vec2(0, instrY - 6.0f),
                             glm::vec2(screenWidth_, 60.0f),
                             glm::vec4(0.03f, 0.05f, 0.08f, 0.92f), true);

        // Instrucciones simplificadas con indicadores visuales
        float xPos = screenWidth_ * 0.5f;
        drawMenuText("W/S NAVEGAR    ENTER INICIAR    ESC SALIR",
                     glm::vec2(xPos, instrY + 12.0f),
                     18.0f, glm::vec4(0.7f, 0.8f, 0.95f, 1.0f), glm::vec2(0.5f, 0.5f));

    }

    glm::vec2 MissionMenu::menuTextSize(const std::string &text, float size, float lineSpacing) const
    {
        if (menuFontReady_)
        {
            return menuFont_.measureText(text, size, lineSpacing);
        }
        if (text.empty())
        {
            return glm::vec2(0.0f);
        }

        float approxWidth = static_cast<float>(text.size()) * size * 0.55f;
        int lines = 1;
        for (char c : text)
        {
            if (c == '\n')
            {
                ++lines;
            }
        }
        float approxHeight = size * static_cast<float>(lines) * lineSpacing;
        return glm::vec2(approxWidth, approxHeight);
    }

    void MissionMenu::drawMenuText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                                   const glm::vec4 &color, const glm::vec2 &anchor, float lineSpacing)
    {
        if (text.empty())
        {
            return;
        }

        glm::vec2 bounds = menuTextSize(text, size, lineSpacing);
        glm::vec2 origin = anchorPoint - glm::vec2(bounds.x * anchor.x, bounds.y * anchor.y);

        if (menuFontReady_)
        {
            menuFont_.drawText(renderer2D_, text, origin, size, color, lineSpacing);
        }
        else
        {
            glm::vec2 fallbackCenter = origin + glm::vec2(bounds.x * 0.5f, bounds.y * 0.5f);
            glm::vec2 charSize(size * 0.6f, size);
            float spacing = size * 0.65f;
            gfx::TextRenderer::drawString(renderer2D_, text, fallbackCenter, charSize, color, spacing);
        }
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
