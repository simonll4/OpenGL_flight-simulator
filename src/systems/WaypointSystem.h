#pragma once

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "flight/data/FlightData.h"
#include "mission/MissionDefinition.h"
#include "mission/MissionRuntime.h"

namespace gfx {
class WaypointRenderer;
}

namespace systems {

class WaypointSystem {
public:
    WaypointSystem();
    ~WaypointSystem();

    void initialize();
    void reset();
    void loadFromMission(const mission::MissionDefinition& mission);

    void update(const glm::vec3& planePos,
                flight::FlightData& flightData,
                mission::MissionRuntime& runtime);

    void render(const glm::mat4& view,
                const glm::mat4& projection,
                const mission::MissionRuntime& runtime) const;

    void skipActiveWaypoint(mission::MissionRuntime& runtime);

    bool empty() const { return waypoints_.empty(); }

private:
    struct WaypointEntry {
        glm::vec3 position;
        std::string name;
        bool captured = false;
    };

    std::unique_ptr<gfx::WaypointRenderer> renderer_;
    std::vector<WaypointEntry> waypoints_;
};

} // namespace systems
