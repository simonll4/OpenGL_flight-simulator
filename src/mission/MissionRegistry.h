/**
 * @file MissionRegistry.h
 * @brief Registro y cargador de misiones desde archivos JSON
 */

#pragma once

#include "MissionDefinition.h"
#include <vector>
#include <string>
#include <memory>

namespace mission
{

    /**
     * @brief Registro centralizado de misiones disponibles
     *
     * Responsabilidades principales:
     * - Parsear archivos `missions.json` en estructuras `MissionDefinition`
     * - Exponer búsquedas rápidas por índice o id (string)
     * - Mantener una copia inmutable para que UI y runtime la consulten sin side-effects
     */
    class MissionRegistry
    {
    public:
        MissionRegistry();
        ~MissionRegistry() = default;

        /**
         * @brief Cargar misiones desde un archivo JSON
         * @param filepath Ruta al archivo JSON con definiciones de misiones
         * @return true si se cargó correctamente
         */
        bool loadFromFile(const std::string &filepath);

        /**
         * @brief Obtener todas las misiones cargadas
         */
        const std::vector<MissionDefinition> &getMissions() const;

        /**
         * @brief Obtener una misión por ID
         * @param id Identificador de la misión
         * @return Puntero a la misión o nullptr si no existe
         */
        const MissionDefinition *getMissionById(const std::string &id) const;

        /**
         * @brief Obtener una misión por índice
         */
        const MissionDefinition *getMissionByIndex(size_t index) const;

        /**
         * @brief Obtener número de misiones cargadas
         */
        size_t getMissionCount() const;

        /**
         * @brief Limpiar todas las misiones
         */
        void clear();

    private:
        std::vector<MissionDefinition> missions_;

        // Helper para parsear JSON manualmente cuando no se usa un parser completo
        MissionDefinition parseMissionFromJson(const std::string &jsonContent);
    };

} // namespace mission
