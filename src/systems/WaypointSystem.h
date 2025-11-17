#pragma once

#include <memory>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "flight/data/FlightData.h"
#include "mission/MissionDefinition.h"
#include "mission/MissionRuntime.h"

namespace gfx
{
    class WaypointRenderer;
}

namespace systems
{

    /**
     * @class WaypointSystem
     * @brief Administra los waypoints de la misión y su representación visual.
     *
     * Sincroniza el estado con `MissionRuntime`, actualiza los datos que el HUD
     * consume (distancias, indicadores) y delega el render a `gfx::WaypointRenderer`.
     */
    class WaypointSystem
    {
    public:
        WaypointSystem();
        ~WaypointSystem();

        /**
         * @brief Reserva recursos del renderer y deja el sistema listo.
         */
        void initialize();

        /**
         * @brief Limpia la lista y marca todos los waypoints como no capturados.
         */
        void reset();

        /**
         * @brief Carga waypoints desde la definición de misión activa.
         */
        void loadFromMission(const mission::MissionDefinition &mission);

        /**
         * @brief Calcula distancias, captura automática y actualiza FlightData.
         */
        void update(const glm::vec3 &planePos,
                    flight::FlightData &flightData,
                    mission::MissionRuntime &runtime);

        /**
         * @brief Dibuja los marcadores 3D del waypoint activo (y próximos).
         */
        void render(const glm::mat4 &view,
                    const glm::mat4 &projection,
                    const mission::MissionRuntime &runtime) const;

        /**
         * @brief Marca manualmente el waypoint activo como completado.
         */
        void skipActiveWaypoint(mission::MissionRuntime &runtime);

        bool empty() const { return waypoints_.empty(); }

    private:
        struct WaypointEntry
        {
            glm::vec3 position;
            std::string name;
            bool captured = false;
        };

        std::unique_ptr<gfx::WaypointRenderer> renderer_;
        std::vector<WaypointEntry> waypoints_;
    };

} // namespace systems
