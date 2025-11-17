/**
 * @file AppState.h
 * @brief Estado de la aplicación (Menu vs Running)
 */

#pragma once

namespace mission
{

    /**
     * @brief Estados posibles de la aplicación
     */
    enum class AppState
    {
        Menu,     // Menú principal de selección
        Planning, // Vista superior para planificar la misión
        Running,  // Ejecutando simulador
        Paused,   // Pausado (opcional para futuro)
        Exiting   // Saliendo
    };

} // namespace mission
