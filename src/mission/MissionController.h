/**
 * @file MissionController.h
 * @brief Mission lifecycle orchestrator.
 */

#pragma once

#include <string>

#include "mission/MissionRegistry.h"
#include "mission/MissionRuntime.h"
#include "mission/MenuStatePersistence.h"

namespace mission
{

    /**
     * @class MissionController
     * @brief Orquesta el ciclo de vida de las misiones: catálogo, selección y ejecución.
     *
     * Encapsula el registro cargado desde disco, el runtime activo y la persistencia
     * del estado del menú para que otras capas (UI, Application) interactúen mediante
     * una API simple.
     */
    class MissionController
    {
    public:
        /**
         * @brief Carga todas las definiciones de misión desde un archivo JSON.
         * @param path Ruta al archivo missions.json.
         * @return true si el parseo y registro fueron exitosos.
         */
        bool loadMissions(const std::string &path);

        MissionRegistry &registry() { return registry_; }
        const MissionRegistry &registry() const { return registry_; }

        MissionRuntime &runtime() { return runtime_; }
        const MissionRuntime &runtime() const { return runtime_; }

        bool hasActiveMission() const { return hasMission_; }
        const MissionDefinition &currentMission() const { return currentMission_; }

        /**
         * @brief Establece la misión seleccionada actualmente y prepara el runtime.
         */
        void setCurrentMission(const MissionDefinition &mission);

        /**
         * @brief Inicializa el contexto de inicio (player spawn, objetivos, clima).
         */
        MissionStartContext startMission();

        /**
         * @brief Reinicia el runtime a estado limpio (se usa al salir de un vuelo).
         */
        void resetRuntime();

        /**
         * @brief Recupera la persistencia del menú (última misión seleccionada, filtros, etc.).
         */
        MenuState loadMenuState() const;

        /**
         * @brief Guarda el estado actual del menú para restaurarlo en la próxima sesión.
         */
        void saveMenuState(const MenuState &state) const;

    private:
        // Contenedor de todas las misiones disponibles
        MissionRegistry registry_;
        // Estado mutable de la misión en ejecución (waypoints, progreso)
        MissionRuntime runtime_;
        MissionDefinition currentMission_{};
        bool hasMission_ = false;
    };

} // namespace mission
