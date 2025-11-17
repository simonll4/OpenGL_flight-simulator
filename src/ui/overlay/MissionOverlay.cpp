/**
 * @file MissionOverlay.cpp
 * @brief Implementación del sistema de overlays de misiones
 */

#include "MissionOverlay.h"
#include "../../mission/MissionDefinition.h"
#include "../../mission/MissionRuntime.h"
#include <GLFW/glfw3.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>

namespace ui {

MissionOverlay::MissionOverlay()
    : screenWidth_(1280)
    , screenHeight_(720)
    , visible_(false)
    , showingBriefing_(false)
    , showingCompletion_(false)
    , readyToFly_(false)
    , completionChoice_(CompletionChoice::None)
    , selectedOption_(0)
    , fadeAlpha_(0.0f)
    , blinkTimer_(0.0f)
    , enterKeyWasPressed_(false)
    , spaceKeyWasPressed_(false)
    , tabKeyWasPressed_(false)
    , upKeyWasPressed_(false)
    , downKeyWasPressed_(false)
{
}

void MissionOverlay::init(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    renderer_.init(screenWidth_, screenHeight_);
    rendererInitialized_ = true;
    
    std::cout << "[MissionOverlay] Inicializado (modo consola)" << std::endl;
}

void MissionOverlay::setScreenSize(int screenWidth, int screenHeight) {
    screenWidth_ = screenWidth;
    screenHeight_ = screenHeight;
    if (rendererInitialized_) {
        renderer_.setScreenSize(screenWidth_, screenHeight_);
    }
}

void MissionOverlay::showBriefing(const mission::MissionDefinition& mission) {
    visible_ = true;
    showingBriefing_ = true;
    showingCompletion_ = false;
    readyToFly_ = false;
    completionChoice_ = CompletionChoice::None;
    fadeAlpha_ = 0.0f;

    missionName_ = mission.name;
    briefingText_ = mission.briefing;

    // Si no hay briefing, pasar directo a ready
    if (briefingText_.empty()) {
        briefingText_ = "Objetivos:\n";
        for (size_t i = 0; i < mission.waypoints.size(); ++i) {
            briefingText_ += "- Navegar a " + mission.waypoints[i].name + "\n";
        }
    }
    
    // Mostrar briefing en consola
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║           BRIEFING DE MISION           ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║ " << mission.name << "\n";
    std::cout << "║\n";
    std::cout << "║ " << briefingText_ << "\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║  [ ENTER ] Para despegar               ║\n";
    std::cout << "╚════════════════════════════════════════╝\n" << std::endl;
}

void MissionOverlay::showCompletionPrompt(const mission::MissionRuntime& runtime) {
    visible_ = true;
    showingBriefing_ = false;
    showingCompletion_ = true;
    completionChoice_ = CompletionChoice::None;
    selectedOption_ = 0;
    readyToFly_ = false;
    fadeAlpha_ = 0.0f;

    missionName_ = runtime.getMission().name;
    metricsText_ = formatMetrics(runtime);
    
    // Mostrar prompt en consola
    std::cout << "\n╔════════════════════════════════════════╗\n";
    std::cout << "║       🎯 MISION COMPLETADA 🎯          ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    const auto& metrics = runtime.getMetrics();
    std::cout << "║ Waypoints: " << metrics.waypointsCaptured << "/" << metrics.totalWaypoints << "                          ║\n";
    std::cout << "║ Tiempo: " << static_cast<int>(metrics.totalTimeSeconds) << "s                             ║\n";
    std::cout << "║ Velocidad: " << static_cast<int>(metrics.averageSpeed) << " kt                      ║\n";
    std::cout << "╠════════════════════════════════════════╣\n";
    std::cout << "║                                        ║\n";
    std::cout << "║  [ SPACE ] Continuar volando           ║\n";
    std::cout << "║  [ TAB   ] Volver al menu              ║\n";
    std::cout << "║  [ ESC   ] Salir del simulador         ║\n";
    std::cout << "║                                        ║\n";
    std::cout << "╚════════════════════════════════════════╝\n" << std::endl;
}

void MissionOverlay::hide() {
    visible_ = false;
    showingBriefing_ = false;
    showingCompletion_ = false;
    readyToFly_ = false;
    completionChoice_ = CompletionChoice::None;
}

void MissionOverlay::render() {
    if (!visible_ || !rendererInitialized_) {
        return;
    }

    renderer_.begin();
    drawBackground(fadeAlpha_);

    if (showingBriefing_) {
        renderBriefing();
    } else if (showingCompletion_) {
        renderCompletion();
    }

    renderer_.end();
}

void MissionOverlay::update(float dt) {
    if (!visible_) {
        return;
    }

    // Fade-in del overlay
    if (fadeAlpha_ < 1.0f) {
        fadeAlpha_ += dt * 2.0f; // 0.5 segundos para fade-in
        if (fadeAlpha_ > 1.0f) fadeAlpha_ = 1.0f;
    }

    // Blink para prompts
    blinkTimer_ += dt;
    if (blinkTimer_ > 6.28f) blinkTimer_ = 0.0f; // 2*PI para ciclo completo
}

bool MissionOverlay::handleInput(GLFWwindow* window) {
    if (!visible_) {
        return false;
    }

    bool actionTaken = false;

    // Detectar teclas con debounce
    bool enterPressed = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
    bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
    bool upPressed = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS;
    bool downPressed = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS;

    // DEBUG: Mostrar estado de teclas
    static int debugCounter = 0;
    if (debugCounter++ % 60 == 0) {  // Cada 60 frames
        std::cout << "[MissionOverlay] visible=" << visible_ 
                  << " briefing=" << showingBriefing_ 
                  << " completion=" << showingCompletion_
                  << " SPACE=" << spacePressed 
                  << " ENTER=" << enterPressed << std::endl;
    }

    // ========================================================================
    // BRIEFING: ENTER para comenzar
    // ========================================================================
    if (showingBriefing_) {
        if (enterPressed && !enterKeyWasPressed_) {
            readyToFly_ = true;
            actionTaken = true;
            std::cout << "[MissionOverlay] Briefing: ENTER presionado - Listo para volar" << std::endl;
        }
    }

    // ========================================================================
    // COMPLETION: Solo SPACE (vuelo libre) y TAB (menú)
    // ========================================================================
    if (showingCompletion_) {
        std::cout << "[MissionOverlay DEBUG] En modo completion, detectando teclas..." << std::endl;
        
        // SPACE para continuar en vuelo libre
        if (spacePressed && !spaceKeyWasPressed_) {
            completionChoice_ = CompletionChoice::FreeFlight;
            selectedOption_ = 1;
            std::cout << "[MissionOverlay] SPACE presionado - Activando vuelo libre" << std::endl;
            actionTaken = true;
        }
        
        // TAB para volver al menú
        if (tabPressed && !tabKeyWasPressed_) {
            completionChoice_ = CompletionChoice::ReturnToMenu;
            selectedOption_ = 0;
            std::cout << "[MissionOverlay] TAB presionado - Volviendo al menú" << std::endl;
            actionTaken = true;
        }
    }

    // Actualizar estado de teclas
    enterKeyWasPressed_ = enterPressed;
    spaceKeyWasPressed_ = spacePressed;
    tabKeyWasPressed_ = tabPressed;
    upKeyWasPressed_ = upPressed;
    downKeyWasPressed_ = downPressed;

    return actionTaken;
}

void MissionOverlay::reset() {
    visible_ = false;
    showingBriefing_ = false;
    showingCompletion_ = false;
    readyToFly_ = false;
    completionChoice_ = CompletionChoice::None;
    selectedOption_ = 0;
    fadeAlpha_ = 0.0f;
    blinkTimer_ = 0.0f;
    enterKeyWasPressed_ = false;
    spaceKeyWasPressed_ = false;
    tabKeyWasPressed_ = false;
    upKeyWasPressed_ = false;
    downKeyWasPressed_ = false;
}

void MissionOverlay::renderBriefing() {
    const float panelW = screenWidth_ * 0.65f;
    const float panelH = screenHeight_ * 0.55f;
    const float panelX = (screenWidth_ - panelW) * 0.5f;
    const float panelY = (screenHeight_ - panelH) * 0.5f;

    drawBox(panelX, panelY, panelW, panelH);

    const glm::vec2 titlePos(panelX + panelW * 0.5f, panelY + 40.0f);
    const glm::vec4 titleColor(0.35f, 0.85f, 1.0f, 1.0f);
    gfx::TextRenderer::drawString(
        renderer_,
        missionName_.empty() ? "MISSION BRIEFING" : missionName_,
        titlePos,
        glm::vec2(16.0f, 26.0f),
        titleColor,
        20.0f);

    auto lines = splitLines(briefingText_);
    if (lines.empty()) {
        lines.emplace_back("Sin briefing disponible.");
    }

    float textY = panelY + 90.0f;
    const glm::vec4 bodyColor(0.85f, 0.92f, 1.0f, 0.95f);
    for (const auto& line : lines) {
        gfx::TextRenderer::drawString(
            renderer_,
            line.empty() ? " " : line,
            glm::vec2(panelX + panelW * 0.5f, textY),
            glm::vec2(9.0f, 14.0f),
            bodyColor,
            11.0f);
        textY += 18.0f;
        if (textY > panelY + panelH - 80.0f) {
            break;
        }
    }

    float blinkAlpha = 0.55f + 0.45f * std::sin(blinkTimer_ * 3.0f);
    glm::vec4 promptColor(0.4f, 1.0f, 0.6f, blinkAlpha);
    gfx::TextRenderer::drawString(
        renderer_,
        "ENTER  LISTO PARA DESPEGAR",
        glm::vec2(panelX + panelW * 0.5f, panelY + panelH - 40.0f),
        glm::vec2(11.0f, 18.0f),
        promptColor,
        14.0f);
}

void MissionOverlay::renderCompletion() {
    const float panelW = screenWidth_ * 0.55f;
    const float panelH = screenHeight_ * 0.45f;
    const float panelX = (screenWidth_ - panelW) * 0.5f;
    const float panelY = (screenHeight_ - panelH) * 0.5f;

    drawBox(panelX, panelY, panelW, panelH);

    const glm::vec4 titleColor(1.0f, 0.9f, 0.5f, 1.0f);
    gfx::TextRenderer::drawString(
        renderer_,
        missionName_.empty() ? "MISION COMPLETADA" : missionName_,
        glm::vec2(panelX + panelW * 0.5f, panelY + 38.0f),
        glm::vec2(15.0f, 24.0f),
        titleColor,
        18.0f);

    auto metricsLines = splitLines(metricsText_);
    float textY = panelY + 85.0f;
    const glm::vec4 metricsColor(0.85f, 0.95f, 1.0f, 0.95f);
    for (const auto& line : metricsLines) {
        gfx::TextRenderer::drawString(
            renderer_,
            line.empty() ? " " : line,
            glm::vec2(panelX + panelW * 0.5f, textY),
            glm::vec2(10.0f, 16.0f),
            metricsColor,
            13.0f);
        textY += 20.0f;
    }

    const float optionY = panelY + panelH - 95.0f;
    const float optionSpacing = 38.0f;
    const float blinkAlpha = 0.55f + 0.45f * std::sin(blinkTimer_ * 4.0f);

    auto drawOption = [&](const std::string& label, float x, float y, bool highlighted) {
        glm::vec4 color = highlighted
                              ? glm::vec4(0.4f, 0.9f, 1.0f, blinkAlpha)
                              : glm::vec4(0.7f, 0.8f, 0.9f, 0.85f);
        gfx::TextRenderer::drawString(
            renderer_,
            label,
            glm::vec2(x, y),
            glm::vec2(11.0f, 18.0f),
            color,
            14.0f);
    };

    const float centerX = panelX + panelW * 0.5f;
    drawOption("TAB  VOLVER AL MENU",
               centerX,
               optionY,
               selectedOption_ == 0);
    drawOption("SPACE  VUELO LIBRE",
               centerX,
               optionY + optionSpacing,
               selectedOption_ == 1);

    gfx::TextRenderer::drawString(
        renderer_,
        "ESC  SALIR DEL SIMULADOR",
        glm::vec2(panelX + panelW * 0.5f, panelY + panelH - 40.0f),
        glm::vec2(9.0f, 14.0f),
        glm::vec4(0.65f, 0.75f, 0.9f, 0.85f),
        11.0f);
}

void MissionOverlay::drawBackground(float alpha) {
    const float clampedAlpha = glm::clamp(alpha, 0.0f, 1.0f);
    renderer_.drawRect(
        glm::vec2(0.0f, 0.0f),
        glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)),
        glm::vec4(0.02f, 0.03f, 0.05f, clampedAlpha * 0.85f),
        true);
}

void MissionOverlay::drawBox(float x, float y, float w, float h) {
    renderer_.drawRect(glm::vec2(x, y), glm::vec2(w, h),
                       glm::vec4(0.06f, 0.1f, 0.16f, 0.95f), true);
    renderer_.drawRect(glm::vec2(x, y), glm::vec2(w, h),
                       glm::vec4(0.3f, 0.8f, 1.0f, 0.9f), false);
}

std::string MissionOverlay::formatMetrics(const mission::MissionRuntime& runtime) {
    const auto& metrics = runtime.getMetrics();
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);
    ss << "WAYPOINTS   " << metrics.waypointsCaptured << "/" << metrics.totalWaypoints << "\n";
    ss << "TIEMPO      " << metrics.totalTimeSeconds << " s\n";
    ss << "VELOCIDAD   " << metrics.averageSpeed << " kt\n";
    ss << "ALTITUD MAX " << metrics.maxAltitude << " ft";
    return ss.str();
}

std::vector<std::string> MissionOverlay::splitLines(const std::string& text) const {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    if (lines.empty()) {
        lines.emplace_back();
    }
    return lines;
}

} // namespace ui
