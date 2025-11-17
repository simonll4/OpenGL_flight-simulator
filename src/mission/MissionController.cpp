#include "mission/MissionController.h"

#include <iostream>

namespace mission
{

    bool MissionController::loadMissions(const std::string &path)
    {
        // Delegar el parseo del JSON al registro central
        bool loaded = registry_.loadFromFile(path);
        if (!loaded)
        {
            std::cerr << "[MissionController] No se pudieron cargar las misiones desde " << path << std::endl;
        }
        return loaded;
    }

    void MissionController::setCurrentMission(const MissionDefinition &mission)
    {
        // Guardar la misión elegida y marcar el flag para habilitar startMission()
        currentMission_ = mission;
        hasMission_ = true;
    }

    MissionStartContext MissionController::startMission()
    {
        if (!hasMission_)
        {
            std::cerr << "[MissionController] No hay misión seleccionada para iniciar" << std::endl;
            return MissionStartContext{};
        }
        // Runtime se encarga de crear entidades, waypoints y estado inicial
        return runtime_.startMission(currentMission_);
    }

    void MissionController::resetRuntime()
    {
        // Restablecer cualquier progreso y limpiar flag de misión activa
        runtime_.reset();
        hasMission_ = false;
    }

    MenuState MissionController::loadMenuState() const
    {
        // Persistencia sencilla basada en archivo JSON dentro de MenuStatePersistence
        return MenuStatePersistence::load();
    }

    void MissionController::saveMenuState(const MenuState &state) const
    {
        // Guardar última selección/estado del menú para sesiones futuras
        MenuStatePersistence::save(state);
    }

} // namespace mission
