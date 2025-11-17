/**
 * @file MissionMenu.h
 * @brief Pantalla de selección de misiones
 */

#pragma once

#include "../../mission/MissionDefinition.h"
#include "../../mission/MissionRegistry.h"
#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"
#include <memory>

extern "C"
{
#include <GLFW/glfw3.h>
}

namespace ui
{

    /**
     * @brief Resultado de la selección del usuario en el menú
     */
    struct MenuResult
    {
        bool missionSelected = false;
        bool exitRequested = false;
        int selectedMissionIndex = -1;
    };

    /**
     * @brief Menú de bienvenida y selección de misiones
     *
     * Muestra una lista de misiones disponibles y permite al usuario
     * seleccionar una para iniciar o salir del simulador.
     */
    class MissionMenu
    {
    public:
        MissionMenu();
        ~MissionMenu() = default;

        /**
         * @brief Inicializar el menú
         * @param registry Registro de misiones disponibles
         * @param screenWidth Ancho de pantalla
         * @param screenHeight Alto de pantalla
         */
        void init(mission::MissionRegistry *registry, int screenWidth, int screenHeight);

        /**
         * @brief Actualizar estado del menú (input handling)
         * @param window Ventana GLFW para capturar input
         * @param deltaTime Tiempo transcurrido desde el último frame
         */
        void update(GLFWwindow *window, float deltaTime);

        /**
         * @brief Renderizar el menú
         */
        void render();

        /**
         * @brief Obtener resultado de la selección
         */
        MenuResult getResult() const;

        /**
         * @brief Resetear el menú para volver a mostrarlo
         */
        void reset();

        /**
         * @brief Actualizar tamaño de pantalla
         */
        void setScreenSize(int width, int height);

        /**
         * @brief Preseleccionar una misión por índice
         */
        void preselectMission(int index);

        /**
         * @brief Acceso al renderer 2D para reutilizarlo en otras vistas
         */
        gfx::Renderer2D *getRenderer() { return &renderer2D_; }

    private:
        mission::MissionRegistry *registry_;
        gfx::Renderer2D renderer2D_;

        int screenWidth_;
        int screenHeight_;
        int selectedIndex_;
        MenuResult result_;

        // Estado de teclas para evitar repetición
        bool enterKeyPressed_;
        bool escKeyPressed_;
        bool upKeyPressed_;
        bool downKeyPressed_;

        // Helpers
        void handleInput(GLFWwindow *window);
        void renderBackground();
        void renderTitle();
        void renderMissionList();
        void renderMissionDetails();
        void renderInstructions();
    };

} // namespace ui
