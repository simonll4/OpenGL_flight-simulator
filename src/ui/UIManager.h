#pragma once

#include <memory>

#include "hud/core/FlightHUD.h"
#include "ui/menu/MissionMenu.h"
#include "ui/menu/MissionPlanner.h"
#include "ui/overlay/MissionOverlay.h"
#include "flight/data/FlightData.h"

struct GLFWwindow;

namespace mission
{
    class MissionRegistry;
    class MissionDefinition;
    class MissionRuntime;
}

namespace ui
{

    /**
     * @class UIManager
     * @brief Punto de orquestación para todos los paneles de UI del simulador.
     *
     * Encapsula la creación, ciclo de vida y comunicación entre:
     * - HUD principal (FlightHUD)
     * - Menú de selección de misión
     * - Planificador de rutas (MissionPlanner)
     * - Overlay in-game (briefing/completion)
     *
     * Expone métodos de actualización/render separados por subsistema para que cada
     * estado del juego invoque sólo lo que necesita.
     */
    class UIManager
    {
    public:
        UIManager();
        ~UIManager();

        /**
         * @brief Inicializa HUD y paneles con dimensiones iniciales y acceso al catálogo de misiones.
         */
        bool initialize(int width, int height, mission::MissionRegistry *registry);

        /**
         * @brief Recalcula layouts cuando cambia el tamaño de la ventana principal.
         */
        void resize(int width, int height);

        // ============================ MENÚ PRINCIPAL ============================
        void updateMenu(GLFWwindow *window, float dt);
        void renderMenu();
        ui::MenuResult getMenuResult() const;
        void resetMenu();
        void preselectMission(int index);

        // ============================ PLANIFICADOR ==============================
        void updatePlanner(GLFWwindow *window, float dt);
        void renderPlanner();
        ui::PlannerResult getPlannerResult() const;
        void resetPlanner();
        void loadPlannerMission(const mission::MissionDefinition &mission);
        mission::MissionDefinition getPlannerMission() const;

        // ============================== OVERLAY =================================
        void updateOverlay(float dt);
        bool handleOverlayInput(GLFWwindow *window);
        bool isOverlayVisible() const;
        bool overlayReadyToFly() const;
        ui::CompletionChoice overlayChoice() const;
        void showBriefing(const mission::MissionDefinition &mission);
        void showCompletionPrompt(const mission::MissionRuntime &runtime);
        void hideOverlay();
        void resetOverlay();
        void renderOverlay();

        // ============================== HUD =====================================
        hud::FlightHUD &hud() { return *hud_; }
        void updateHUD(const flight::FlightData &data);
        void renderHUD();

    private:
        // Objetos propietarios de cada capa de interfaz
        std::unique_ptr<hud::FlightHUD> hud_;
        std::unique_ptr<ui::MissionMenu> menu_;
        std::unique_ptr<ui::MissionPlanner> planner_;
        std::unique_ptr<ui::MissionOverlay> overlay_;

        // Dependencias externas y parámetros de layout
        mission::MissionRegistry *registry_ = nullptr;
        int screenWidth_ = 1280;
        int screenHeight_ = 720;
    };

} // namespace ui
