#pragma once

#include <string>

#include "mission/MissionRegistry.h"
#include "mission/MissionRuntime.h"
#include "mission/MenuStatePersistence.h"

namespace mission {

class MissionController {
public:
    bool loadMissions(const std::string& path);

    MissionRegistry& registry() { return registry_; }
    const MissionRegistry& registry() const { return registry_; }

    MissionRuntime& runtime() { return runtime_; }
    const MissionRuntime& runtime() const { return runtime_; }

    bool hasActiveMission() const { return hasMission_; }
    const MissionDefinition& currentMission() const { return currentMission_; }

    void setCurrentMission(const MissionDefinition& mission);
    MissionStartContext startMission();
    void resetRuntime();

    MenuState loadMenuState() const;
    void saveMenuState(const MenuState& state) const;

private:
    MissionRegistry registry_;
    MissionRuntime runtime_;
    MissionDefinition currentMission_{};
    bool hasMission_ = false;
};

} // namespace mission
