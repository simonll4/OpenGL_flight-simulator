#include "mission/MissionController.h"

#include <iostream>

namespace mission {

bool MissionController::loadMissions(const std::string& path) {
    bool loaded = registry_.loadFromFile(path);
    if (!loaded) {
        std::cerr << "[MissionController] No se pudieron cargar las misiones desde " << path << std::endl;
    }
    return loaded;
}

void MissionController::setCurrentMission(const MissionDefinition& mission) {
    currentMission_ = mission;
    hasMission_ = true;
}

MissionStartContext MissionController::startMission() {
    if (!hasMission_) {
        std::cerr << "[MissionController] No hay misión seleccionada para iniciar" << std::endl;
        return MissionStartContext{};
    }
    return runtime_.startMission(currentMission_);
}

void MissionController::resetRuntime() {
    runtime_.reset();
    hasMission_ = false;
}

MenuState MissionController::loadMenuState() const {
    return MenuStatePersistence::load();
}

void MissionController::saveMenuState(const MenuState& state) const {
    MenuStatePersistence::save(state);
}

} // namespace mission
