/**
 * @file MissionRegistry.h
 * @brief Registro y cargador de misiones desde archivos JSON
 */

#pragma once

#include "MissionDefinition.h"
#include <vector>
#include <string>
#include <memory>

namespace mission {

/**
 * @brief Registro centralizado de misiones disponibles
 * 
 * Carga misiones desde archivos JSON y las mantiene en memoria
 * para selección del usuario.
 */
class MissionRegistry {
public:
    MissionRegistry();
    ~MissionRegistry() = default;
    
    /**
     * @brief Cargar misiones desde un archivo JSON
     * @param filepath Ruta al archivo JSON con definiciones de misiones
     * @return true si se cargó correctamente
     */
    bool loadFromFile(const std::string& filepath);
    
    /**
     * @brief Obtener todas las misiones cargadas
     */
    const std::vector<MissionDefinition>& getMissions() const;
    
    /**
     * @brief Obtener una misión por ID
     * @param id Identificador de la misión
     * @return Puntero a la misión o nullptr si no existe
     */
    const MissionDefinition* getMissionById(const std::string& id) const;
    
    /**
     * @brief Obtener una misión por índice
     */
    const MissionDefinition* getMissionByIndex(size_t index) const;
    
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
    
    // Helper para parsear JSON manualmente (simple implementation)
    MissionDefinition parseMissionFromJson(const std::string& jsonContent);
};

} // namespace mission
