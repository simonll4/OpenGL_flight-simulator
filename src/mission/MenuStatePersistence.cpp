/**
 * @file MenuStatePersistence.cpp
 * @brief Implementación de la persistencia del estado del menú
 */

#include "MenuStatePersistence.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace mission {

bool MenuStatePersistence::save(const MenuState& state, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: No se pudo guardar el estado del menú en: " << filepath << std::endl;
        return false;
    }
    
    // Escribir JSON simple
    file << "{\n";
    file << "  \"lastMissionId\": \"" << state.lastMissionId << "\",\n";
    file << "  \"lastMissionIndex\": " << state.lastMissionIndex << "\n";
    file << "}\n";
    
    file.close();
    std::cout << "✓ Estado del menú guardado" << std::endl;
    return true;
}

MenuState MenuStatePersistence::load(const std::string& filepath) {
    MenuState state;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        // No es un error crítico, simplemente no hay estado previo
        std::cout << "Info: No se encontró estado previo del menú" << std::endl;
        return state;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    // Parser JSON simple
    size_t idPos = content.find("\"lastMissionId\"");
    if (idPos != std::string::npos) {
        size_t colonPos = content.find(':', idPos);
        size_t valueStart = content.find('"', colonPos);
        size_t valueEnd = content.find('"', valueStart + 1);
        if (valueStart != std::string::npos && valueEnd != std::string::npos) {
            state.lastMissionId = content.substr(valueStart + 1, valueEnd - valueStart - 1);
        }
    }
    
    size_t indexPos = content.find("\"lastMissionIndex\"");
    if (indexPos != std::string::npos) {
        size_t colonPos = content.find(':', indexPos);
        size_t valueStart = colonPos + 1;
        
        // Saltar espacios
        while (valueStart < content.length() && 
               (content[valueStart] == ' ' || content[valueStart] == '\n' || content[valueStart] == '\t'))
            valueStart++;
        
        size_t valueEnd = valueStart;
        while (valueEnd < content.length() && 
               content[valueEnd] != ',' && content[valueEnd] != '}' && content[valueEnd] != '\n')
            valueEnd++;
        
        std::string valueStr = content.substr(valueStart, valueEnd - valueStart);
        
        // Remover espacios finales
        while (!valueStr.empty() && 
               (valueStr.back() == ' ' || valueStr.back() == '\n' || valueStr.back() == '\t'))
            valueStr.pop_back();
        
        try {
            state.lastMissionIndex = std::stoi(valueStr);
        } catch (...) {
            state.lastMissionIndex = 0;
        }
    }
    
    std::cout << "✓ Estado del menú cargado (última misión: " << state.lastMissionId << ")" << std::endl;
    return state;
}

} // namespace mission
