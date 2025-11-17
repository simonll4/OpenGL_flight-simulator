/**
 * @file MenuStatePersistence.h
 * @brief Persistencia del estado del menú (última misión seleccionada)
 */

#pragma once

#include <string>

namespace mission {

/**
 * @brief Estado persistente del menú
 */
struct MenuState {
    std::string lastMissionId = "";
    int lastMissionIndex = 0;
};

/**
 * @brief Gestor de persistencia del estado del menú
 */
class MenuStatePersistence {
public:
    /**
     * @brief Guardar el estado del menú
     * @param state Estado a guardar
     * @param filepath Ruta del archivo (por defecto: menu_state.json)
     * @return true si se guardó correctamente
     */
    static bool save(const MenuState& state, const std::string& filepath = "menu_state.json");
    
    /**
     * @brief Cargar el estado del menú
     * @param filepath Ruta del archivo (por defecto: menu_state.json)
     * @return Estado cargado (valores por defecto si no existe)
     */
    static MenuState load(const std::string& filepath = "menu_state.json");
};

} // namespace mission
