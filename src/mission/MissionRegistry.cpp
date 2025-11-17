/**
 * @file MissionRegistry.cpp
 * @brief Implementación del registro de misiones
 */

#include "MissionRegistry.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace mission {

MissionRegistry::MissionRegistry() {
}

bool MissionRegistry::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo de misiones: " << filepath << std::endl;
        return false;
    }
    
    // Leer todo el contenido del archivo
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // Parser JSON simple
    // Esperamos formato: { "missions": [ {...}, {...}, ... ] }
    
    // Encontrar el array de misiones
    size_t missionsStart = content.find("\"missions\"");
    if (missionsStart == std::string::npos) {
        std::cerr << "Error: No se encontró el campo 'missions' en el JSON" << std::endl;
        return false;
    }
    
    // Encontrar el inicio del array
    size_t arrayStart = content.find('[', missionsStart);
    size_t arrayEnd = content.rfind(']');
    
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        std::cerr << "Error: Formato JSON inválido" << std::endl;
        return false;
    }
    
    // Extraer cada objeto de misión
    std::string arrayContent = content.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
    
    // Parsear misiones individuales (buscar objetos {...})
    size_t pos = 0;
    while (pos < arrayContent.length()) {
        size_t objStart = arrayContent.find('{', pos);
        if (objStart == std::string::npos) break;
        
        // Encontrar el cierre del objeto (contando llaves)
        int braceCount = 1;
        size_t objEnd = objStart + 1;
        while (objEnd < arrayContent.length() && braceCount > 0) {
            if (arrayContent[objEnd] == '{') braceCount++;
            else if (arrayContent[objEnd] == '}') braceCount--;
            objEnd++;
        }
        
        std::string missionJson = arrayContent.substr(objStart, objEnd - objStart);
        MissionDefinition mission = parseMissionFromJson(missionJson);
        
        if (!mission.id.empty()) {
            missions_.push_back(mission);
            std::cout << "✓ Misión cargada: " << mission.name << " [" << mission.id << "]" << std::endl;
        }
        
        pos = objEnd;
    }
    
    std::cout << "Total de misiones cargadas: " << missions_.size() << std::endl;
    return !missions_.empty();
}

// Helper simple para extraer valores de campos JSON
static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return "";
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return "";
    
    size_t valueStart = json.find('"', colonPos);
    if (valueStart == std::string::npos) return "";
    
    size_t valueEnd = json.find('"', valueStart + 1);
    if (valueEnd == std::string::npos) return "";
    
    return json.substr(valueStart + 1, valueEnd - valueStart - 1);
}

static float extractJsonFloat(const std::string& json, const std::string& key, float defaultValue = 0.0f) {
    std::string searchKey = "\"" + key + "\"";
    size_t keyPos = json.find(searchKey);
    if (keyPos == std::string::npos) return defaultValue;
    
    size_t colonPos = json.find(':', keyPos);
    if (colonPos == std::string::npos) return defaultValue;
    
    // Saltar espacios
    size_t valueStart = colonPos + 1;
    while (valueStart < json.length() && (json[valueStart] == ' ' || json[valueStart] == '\n' || json[valueStart] == '\t'))
        valueStart++;
    
    // Leer hasta coma o cierre
    size_t valueEnd = valueStart;
    while (valueEnd < json.length() && json[valueEnd] != ',' && json[valueEnd] != '}' && json[valueEnd] != ']')
        valueEnd++;
    
    std::string valueStr = json.substr(valueStart, valueEnd - valueStart);
    
    // Remover espacios finales
    while (!valueStr.empty() && (valueStr.back() == ' ' || valueStr.back() == '\n' || valueStr.back() == '\t'))
        valueStr.pop_back();
    
    try {
        return std::stof(valueStr);
    } catch (...) {
        return defaultValue;
    }
}

static int extractJsonInt(const std::string& json, const std::string& key, int defaultValue = 0) {
    return static_cast<int>(extractJsonFloat(json, key, static_cast<float>(defaultValue)));
}

MissionDefinition MissionRegistry::parseMissionFromJson(const std::string& jsonContent) {
    MissionDefinition mission;
    
    mission.id = extractJsonString(jsonContent, "id");
    mission.name = extractJsonString(jsonContent, "name");
    mission.description = extractJsonString(jsonContent, "description");
    mission.briefing = extractJsonString(jsonContent, "briefing");
    mission.category = extractJsonString(jsonContent, "category");
    mission.difficulty = extractJsonInt(jsonContent, "difficulty", 1);
    mission.thumbnailPath = extractJsonString(jsonContent, "thumbnailPath");
    
    // Parsear startPosition
    size_t startPosKey = jsonContent.find("\"startPosition\"");
    if (startPosKey != std::string::npos) {
        size_t objStart = jsonContent.find('{', startPosKey);
        size_t objEnd = jsonContent.find('}', objStart);
        if (objStart != std::string::npos && objEnd != std::string::npos) {
            std::string posObj = jsonContent.substr(objStart, objEnd - objStart + 1);
            mission.startPosition.x = extractJsonFloat(posObj, "x", 0.0f);
            mission.startPosition.y = extractJsonFloat(posObj, "y", 1500.0f);
            mission.startPosition.z = extractJsonFloat(posObj, "z", 0.0f);
        }
    }
    
    // Parsear waypoints
    size_t waypointsKey = jsonContent.find("\"waypoints\"");
    if (waypointsKey != std::string::npos) {
        size_t arrayStart = jsonContent.find('[', waypointsKey);
        size_t arrayEnd = jsonContent.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string waypointsArray = jsonContent.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            
            // Parsear cada waypoint
            size_t pos = 0;
            while (pos < waypointsArray.length()) {
                size_t wpStart = waypointsArray.find('{', pos);
                if (wpStart == std::string::npos) break;
                
                int braceCount = 1;
                size_t wpEnd = wpStart + 1;
                while (wpEnd < waypointsArray.length() && braceCount > 0) {
                    if (waypointsArray[wpEnd] == '{') braceCount++;
                    else if (waypointsArray[wpEnd] == '}') braceCount--;
                    wpEnd++;
                }
                
                std::string wpJson = waypointsArray.substr(wpStart, wpEnd - wpStart);
                
                WaypointDef wp;
                wp.name = extractJsonString(wpJson, "name");
                
                // Parsear position
                size_t posKey = wpJson.find("\"position\"");
                if (posKey != std::string::npos) {
                    size_t pObjStart = wpJson.find('{', posKey);
                    size_t pObjEnd = wpJson.find('}', pObjStart);
                    if (pObjStart != std::string::npos && pObjEnd != std::string::npos) {
                        std::string posObj = wpJson.substr(pObjStart, pObjEnd - pObjStart + 1);
                        wp.position.x = extractJsonFloat(posObj, "x", 0.0f);
                        wp.position.y = extractJsonFloat(posObj, "y", 1500.0f);
                        wp.position.z = extractJsonFloat(posObj, "z", 0.0f);
                    }
                }
                
                mission.waypoints.push_back(wp);
                pos = wpEnd;
            }
        }
    }
    
    // Parsear environment (opcional)
    size_t envKey = jsonContent.find("\"environment\"");
    if (envKey != std::string::npos) {
        size_t objStart = jsonContent.find('{', envKey);
        size_t objEnd = jsonContent.find('}', objStart);
        if (objStart != std::string::npos && objEnd != std::string::npos) {
            std::string envObj = jsonContent.substr(objStart, objEnd - objStart + 1);
            mission.environment.timeOfDay = extractJsonString(envObj, "timeOfDay");
            mission.environment.weather = extractJsonString(envObj, "weather");
            mission.environment.windSpeed = extractJsonFloat(envObj, "windSpeed", 0.0f);
            mission.environment.windDirection = extractJsonFloat(envObj, "windDirection", 0.0f);
        }
    }
    
    return mission;
}

const std::vector<MissionDefinition>& MissionRegistry::getMissions() const {
    return missions_;
}

const MissionDefinition* MissionRegistry::getMissionById(const std::string& id) const {
    for (const auto& mission : missions_) {
        if (mission.id == id) {
            return &mission;
        }
    }
    return nullptr;
}

const MissionDefinition* MissionRegistry::getMissionByIndex(size_t index) const {
    if (index < missions_.size()) {
        return &missions_[index];
    }
    return nullptr;
}

size_t MissionRegistry::getMissionCount() const {
    return missions_.size();
}

void MissionRegistry::clear() {
    missions_.clear();
}

} // namespace mission
