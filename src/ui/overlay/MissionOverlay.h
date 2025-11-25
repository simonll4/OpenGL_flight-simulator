/**
 * @file MissionOverlay.h
 * @brief Sistema de overlays para briefing y finalización de misiones
 *
 * Renderiza mensajes modales durante las fases Briefing y Completed:
 * - Briefing: Muestra título, objetivos y prompt de inicio
 * - Completion: Muestra resumen y opciones (menú o vuelo libre)
 */

#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"
#include "../../gfx/rendering/TrueTypeFont.h"

// Forward declarations para evitar includes
struct GLFWwindow;

namespace mission
{
    struct MissionDefinition;
    class MissionRuntime;
}

namespace ui
{

    /**
     * @brief Opciones de finalización de misión
     */
    enum class CompletionChoice
    {
        None,         // Aún no se eligió
        ReturnToMenu, // Volver al menú (ENTER)
        FreeFlight    // Continuar en vuelo libre (SPACE)
    };

    /**
     * @brief Overlay modal para misiones
     */
    class MissionOverlay
    {
    public:
        MissionOverlay();
        ~MissionOverlay() = default;

        /**
         * @brief Inicializa recursos gráficos
         */
        void init(int screenWidth, int screenHeight);
        void setScreenSize(int screenWidth, int screenHeight);

        /**
         * @brief Muestra el briefing de una misión
         */
        void showBriefing(const mission::MissionDefinition &mission);

        /**
         * @brief Muestra el prompt de finalización
         */
        void showCompletionPrompt(const mission::MissionRuntime &runtime);

        /**
         * @brief Oculta el overlay
         */
        void hide();

        /**
         * @brief Renderiza el overlay
         */
        void render();

        /**
         * @brief Actualiza animaciones
         */
        void update(float dt);

        /**
         * @brief Procesa input del usuario
         * @return true si se procesó alguna acción
         */
        bool handleInput(GLFWwindow *window);

        /**
         * @brief Verifica si el piloto está listo para volar (briefing)
         */
        bool readyToFly() const { return readyToFly_; }

        /**
         * @brief Verifica si el overlay está visible
         */
        bool isVisible() const { return visible_; }

        /**
         * @brief Obtiene la elección del usuario en pantalla de finalización
         */
        CompletionChoice getCompletionChoice() const { return completionChoice_; }

        /**
         * @brief Resetea el estado del overlay
         */
        void reset();

    private:
        // Dimensiones de pantalla
        int screenWidth_;
        int screenHeight_;
        bool rendererInitialized_ = false;
        gfx::Renderer2D renderer_;

        // Estado
        bool visible_;
        bool showingBriefing_;
        bool showingCompletion_;
        bool readyToFly_;
        CompletionChoice completionChoice_;
        int selectedOption_; // 0 = menu, 1 = free flight

        // Contenido
        std::string missionName_;
        std::string briefingText_;
        std::string metricsText_;

        // Animación
        float fadeAlpha_;
        float blinkTimer_;

        // Control de input
        bool enterKeyWasPressed_;
        bool spaceKeyWasPressed_;
        bool tabKeyWasPressed_;
        bool upKeyWasPressed_;
        bool downKeyWasPressed_;

        // Helpers
        void renderBriefing();
        void renderCompletion();
        void drawBackground(float alpha);
        void drawBox(float x, float y, float w, float h);
        std::string formatMetrics(const mission::MissionRuntime &runtime);
        std::vector<std::string> splitLines(const std::string &text) const;
        glm::vec2 overlayTextSize(const std::string &text, float size, float lineSpacing = 1.0f) const;
        void drawOverlayText(const std::string &text, const glm::vec2 &anchorPoint, float size,
                             const glm::vec4 &color, const glm::vec2 &anchor = glm::vec2(0.5f),
                             float lineSpacing = 1.0f);

        gfx::TrueTypeFont overlayFont_;
        bool overlayFontReady_ = false;
    };

} // namespace ui
