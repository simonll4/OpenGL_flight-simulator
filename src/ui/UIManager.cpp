#include "ui/UIManager.h"

#include <stdexcept>

namespace ui
{

    UIManager::UIManager() = default;
    UIManager::~UIManager() = default;

    bool UIManager::initialize(int width, int height, mission::MissionRegistry *registry)
    {
        registry_ = registry;
        screenWidth_ = width;
        screenHeight_ = height;

        // HUD principal: crea layout por defecto y lo inicializa con la resolución actual
        hud_ = std::make_unique<hud::FlightHUD>();
        hud_->init(width, height);
        hud_->setLayout("classic");

        // Menú se alimenta del registro de misiones para listar escenarios disponibles
        menu_ = std::make_unique<ui::MissionMenu>();
        menu_->init(registry_, width, height);

        // El planner reutiliza el renderer del menú para compartir recursos (fuentes, texturas)
        planner_ = std::make_unique<ui::MissionPlanner>();
        planner_->init(width, height, menu_->getRenderer());

        // Overlay se encarga de briefing/completion sobre la vista 3D
        overlay_ = std::make_unique<ui::MissionOverlay>();
        overlay_->init(width, height);

        return true;
    }

    void UIManager::resize(int width, int height)
    {
        // Guardar resolución actual para futuros cálculos de layout
        screenWidth_ = width;
        screenHeight_ = height;
        if (hud_)
        {
            // FlightHUD recalcula grillas y posiciones relativas
            hud_->setScreenSize(width, height);
        }
        if (menu_)
        {
            // Los menús dependen de la resolución para su malla de navegación
            menu_->setScreenSize(width, height);
        }
        if (planner_)
        {
            planner_->setScreenSize(width, height);
        }
        if (overlay_)
        {
            overlay_->setScreenSize(width, height);
        }
    }

    void UIManager::updateMenu(GLFWwindow *window, float dt)
    {
        // Reenvía entrada y delta de tiempo al menú de selección
        if (menu_)
        {
            menu_->update(window, dt);
        }
    }

    void UIManager::renderMenu()
    {
        // El menú usa su propio Renderer2D, sólo hay que detonar el draw call
        if (menu_)
        {
            menu_->render();
        }
    }

    ui::MenuResult UIManager::getMenuResult() const
    {
        // Devuelve el resultado del último frame (start, exit, etc.)
        if (menu_)
        {
            return menu_->getResult();
        }
        return ui::MenuResult{};
    }

    void UIManager::resetMenu()
    {
        // Limpia selecciones y estados transitorios del menú
        if (menu_)
        {
            menu_->reset();
        }
    }

    void UIManager::preselectMission(int index)
    {
        // Permite marcar una misión cuando venimos del planner u otro flujo
        if (menu_)
        {
            menu_->preselectMission(index);
        }
    }

    void UIManager::updatePlanner(GLFWwindow *window, float dt)
    {
        // El planner responde a input para manipular waypoints
        if (planner_)
        {
            planner_->update(window, dt);
        }
    }

    void UIManager::renderPlanner()
    {
        // Dibuja mapa, waypoints y UI adicional
        if (planner_)
        {
            planner_->render();
        }
    }

    ui::PlannerResult UIManager::getPlannerResult() const
    {
        // Reporta la acción del usuario (aceptar plan, regresar, etc.)
        if (planner_)
        {
            return planner_->getResult();
        }
        return ui::PlannerResult{};
    }

    void UIManager::resetPlanner()
    {
        // Borra ruta y estados temporales al abandonar el planner
        if (planner_)
        {
            planner_->reset();
        }
    }

    void UIManager::loadPlannerMission(const mission::MissionDefinition &mission)
    {
        // Precarga una misión para editarla dentro del planner
        if (planner_)
        {
            planner_->loadMission(mission);
        }
    }

    mission::MissionDefinition UIManager::getPlannerMission() const
    {
        // Devuelve la misión editada actualmente (copia)
        if (planner_)
        {
            return planner_->getMission();
        }
        return mission::MissionDefinition();
    }

    void UIManager::updateOverlay(float dt)
    {
        // Overlay sólo necesita delta de tiempo para sus animaciones
        if (overlay_)
        {
            overlay_->update(dt);
        }
    }

    bool UIManager::handleOverlayInput(GLFWwindow *window)
    {
        // Maneja clicks/teclas cuando el overlay está visible
        if (overlay_)
        {
            return overlay_->handleInput(window);
        }
        return false;
    }

    bool UIManager::isOverlayVisible() const
    {
        return overlay_ && overlay_->isVisible();
    }

    bool UIManager::overlayReadyToFly() const
    {
        return overlay_ && overlay_->readyToFly();
    }

    ui::CompletionChoice UIManager::overlayChoice() const
    {
        if (overlay_)
        {
            return overlay_->getCompletionChoice();
        }
        return ui::CompletionChoice::None;
    }

    void UIManager::showBriefing(const mission::MissionDefinition &mission)
    {
        // Muestra briefing previo al vuelo con datos de la misión seleccionada
        if (overlay_)
        {
            overlay_->showBriefing(mission);
        }
    }

    void UIManager::showCompletionPrompt(const mission::MissionRuntime &runtime)
    {
        // Presenta resultados del vuelo actual y opciones de continuación
        if (overlay_)
        {
            overlay_->showCompletionPrompt(runtime);
        }
    }

    void UIManager::hideOverlay()
    {
        // Esconde overlay sin destruirlo (permite volver a mostrarlo rápido)
        if (overlay_)
        {
            overlay_->hide();
        }
    }

    void UIManager::resetOverlay()
    {
        // Limpia estado interno (selecciones previas, textos, etc.)
        if (overlay_)
        {
            overlay_->reset();
        }
    }

    void UIManager::renderOverlay()
    {
        // Dibuja paneles semitransparentes sobre la escena 3D
        if (overlay_)
        {
            overlay_->render();
        }
    }

    void UIManager::updateHUD(const flight::FlightData &data)
    {
        // Propaga los datos de vuelo más recientes al HUD
        if (hud_)
        {
            hud_->update(data);
        }
    }

    void UIManager::renderHUD()
    {
        // Renderiza el HUD por encima del resto de elementos
        if (hud_)
        {
            hud_->render();
        }
    }

} // namespace ui
