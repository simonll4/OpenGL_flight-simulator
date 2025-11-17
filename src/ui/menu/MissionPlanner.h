/**
 * @file MissionPlanner.h
 * @brief Editor de misiones y planificador de waypoints
 *
 * Esta clase proporciona una interfaz de usuario interactiva para
 * definir la ruta de vuelo que seguirá el avión en la simulación. La
 * interfaz se compone de tres paneles: un mapa ortográfico donde se
 * insertan y arrastran los waypoints, un gráfico de altitud en función
 * de la distancia para visualizar el perfil vertical y ajustar alturas,
 * y un storyboard de tarjetas que resume cada segmento de la misión.
 *
 * El planificador se encarga de convertir las posiciones en el mundo
 * (coordenadas XZ) a coordenadas de pantalla, de gestionar los eventos
 * de entrada (ratón y teclado) y de actualizar la estructura
 * MissionDefinition con los cambios realizados por el usuario. El
 * objetivo es ofrecer una herramienta sencilla pero profesional para
 * configurar misiones antes de lanzar el simulador de vuelo.
 */

#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "../../mission/MissionDefinition.h"
#include "../../gfx/rendering/Renderer2D.h"
#include "../../gfx/rendering/TextRenderer.h"

// Reenvío de la estructura GLFWwindow para no incluir GLFW en el header
struct GLFWwindow;

namespace ui {

/**
 * @brief Estructura de resultados devueltos por el planificador
 *
 * Cuando el usuario interactúa con la interfaz, puede solicitar
 * iniciar la misión (startRequested) o cancelar y volver al menú
 * principal (cancelRequested). Estos flags son consumidos por el
 * bucle principal de la aplicación.
 */
struct PlannerResult {
    bool startRequested = false;
    bool cancelRequested = false;
};

/**
 * @brief Planificador de misiones con edición gráfica de waypoints
 *
 * La clase MissionPlanner encapsula toda la lógica de la pantalla de
 * planificación. Utiliza un Renderer2D compartido para dibujar la
 * interfaz y un MissionDefinition como almacenamiento de la misión
 * actual en construcción. El usuario puede añadir, mover y eliminar
 * waypoints en el mapa, ajustar sus altitudes mediante comandos de
 * teclado y visualizar el perfil de altitud y la distancia total.
 */
class MissionPlanner {
public:
    MissionPlanner();

    /**
     * @brief Inicializa el planificador
     * @param screenWidth Ancho de pantalla en píxeles
     * @param screenHeight Alto de pantalla en píxeles
     * @param sharedRenderer Puntero a un Renderer2D ya inicializado
     */
    void init(int screenWidth, int screenHeight, gfx::Renderer2D* sharedRenderer);

    /**
     * @brief Ajusta el tamaño de pantalla
     *
     * Debe llamarse cuando la ventana se redimensiona para recalcular
     * disposiciones y escalados.
     */
    void setScreenSize(int width, int height);

    /**
     * @brief Carga una misión existente para editar
     * @param mission Definición de misión a editar
     *
     * Se copia la misión suministrada en el planificador y se calcula
     * automáticamente el encuadre del mapa en base a la posición de
     * salida y los waypoints existentes. También se reinician todos
     * los estados de interacción.
     */
    void loadMission(const mission::MissionDefinition& mission);

    /**
     * @brief Actualiza el estado del planificador
     * @param window Puntero a la ventana GLFW para leer entradas
     * @param deltaTime Tiempo en segundos transcurrido desde el último frame
     */
    void update(GLFWwindow* window, float deltaTime);

    /**
     * @brief Dibuja la interfaz del planificador
     */
    void render();

    /**
     * @brief Devuelve el estado de la interacción del planificador
     */
    PlannerResult getResult() const { return result_; }

    /**
     * @brief Obtiene una referencia constante a la misión en edición
     */
    const mission::MissionDefinition& getMission() const { return workingMission_; }

    /**
     * @brief Restablece el planificador a un estado neutro
     *
     * Limpia los indicadores de inicio/cancelación y anula las
     * interacciones en curso. No elimina ni altera los waypoints.
     */
    void reset();

    // === Funcionalidades adicionales ===
    /**
     * @brief Genera un informe de la misión (no implementado)
     */
    void generateMissionReport();

    /**
     * @brief Genera automáticamente un patrón de waypoints
     * @param pattern Selección de patrón (0: cuadrado, 1: círculo, etc.)
     */
    void autoGenerateWaypoints(int pattern);

    /**
     * @brief Valida si la misión cumple requisitos mínimos
     * @return true si hay al menos un waypoint definido
     */
    bool validateMission() const;

    /**
     * @brief Guarda la misión en un archivo JSON (no implementado)
     */
    void saveMissionToFile(const std::string& path);

    /**
     * @brief Carga una misión desde un archivo JSON (no implementado)
     */
    void loadMissionFromFile(const std::string& path);

private:
    // === Métodos internos ===
    void updateLayout();
    void handleKeyboardInput(GLFWwindow* window, float deltaTime);
    void handleMouseInput(GLFWwindow* window);
    void updateStoryboardHover();
    void renderBackground();
    void renderToolbar();
    void renderMap();
    void renderProfile();
    void renderStoryboard();
    void renderInstructions();
    void renderStartButton();

    glm::vec2 mapWorldToScreen(const glm::vec3& world) const;
    glm::vec3 mapScreenToWorld(const glm::vec2& screen) const;
    int findWaypointNear(const glm::vec2& screen, float threshold) const;
    size_t maxVisibleCards() const;
    bool cardRect(size_t index, glm::vec2& pos, glm::vec2& size) const;
    bool cursorInsideMap() const;
    bool cursorInsideProfile() const;
    bool cursorInsideStoryboard() const;
    float computeMissionLength() const;

    // === Miembros ===
    gfx::Renderer2D* renderer_ = nullptr;      ///< Renderizador 2D compartido
    mission::MissionDefinition workingMission_; ///< Misión en construcción
    PlannerResult result_;                      ///< Resultado de la última interacción

    int screenWidth_ = 1280; ///< Ancho de pantalla actual
    int screenHeight_ = 720; ///< Alto de pantalla actual

    // Rectángulos de la interfaz
    glm::vec2 mapOrigin_ = glm::vec2(0.0f);
    glm::vec2 mapSize_ = glm::vec2(1.0f);
    glm::vec2 profileOrigin_ = glm::vec2(0.0f);
    glm::vec2 profileSize_ = glm::vec2(1.0f);
    glm::vec2 storyboardOrigin_ = glm::vec2(0.0f);
    glm::vec2 storyboardSize_ = glm::vec2(1.0f);
    glm::vec2 buttonPos_ = glm::vec2(0.0f);
    glm::vec2 buttonSize_ = glm::vec2(220.0f, 56.0f);
    glm::vec2 cursorPos_ = glm::vec2(0.0f);

    // Variables de navegación y escala
    glm::vec2 mapCenter_ = glm::vec2(0.0f); ///< Punto central en el mundo representado en pantalla
    float mapHalfExtent_ = 10000.0f;        ///< Extensión máxima visible en cada eje (metros)
    float defaultAltitude_ = 1500.0f;       ///< Altitud predeterminada para nuevos waypoints

    int selectedIndex_ = -1; ///< Índice de waypoint seleccionado para edición
    bool draggingXY_ = false; ///< Se está arrastrando un waypoint en el plano XZ
    int draggingIndex_ = -1;   ///< Índice del waypoint que se arrastra

    // Flags de estado de botones/teclas
    bool leftMouseHeld_ = false;
    bool rightMouseHeld_ = false;
    bool enterHeld_ = false;
    bool escHeld_ = false;
    bool rHeld_ = false;
    bool fHeld_ = false;

    // Estados de hover y selección
    bool startButtonHovered_ = false;
    int hoveredCardIndex_ = -1;

    // Cachés de renderizado
    float cachedMissionLength_ = 0.0f;
    float storyboardCardHeight_ = 90.0f;
    float panSpeed_ = 200.0f; ///< Velocidad de paneo del mapa en metros/segundo
};

} // namespace ui