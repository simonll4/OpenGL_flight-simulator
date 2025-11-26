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
        Planning, // Vista para planificar la misión
        Running,  // Ejecutando simulador
        Paused,   // Pausado (sin implementar)
        Exiting   // Saliendo
    };

} // namespace mission
