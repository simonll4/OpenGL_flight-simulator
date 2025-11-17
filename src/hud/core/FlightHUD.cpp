/**
 * =============================================================================
 * PROPÓSITO:
 * Este archivo gestiona TODOS los instrumentos del HUD del simulador de vuelo.
 * =============================================================================
 *
 * ARQUITECTURA:
 * El FlightHUD es el coordinador central que:
 * 1. Crea y gestiona todos los instrumentos de vuelo
 * 2. Les pasa los datos actualizados del avión (FlightData)
 * 3. Coordina el renderizado en el orden correcto
 * 4. Maneja el layout y posicionamiento de cada instrumento
 *
 * INSTRUMENTOS:
 * - Altimeter
 * - AttitudeIndicator
 * - AirspeedIndicator
 * - HeadingIndicator
 * - VerticalSpeedIndicator
 * - TurnCoordinator
 *
 * =============================================================================
 * GUÍA PARA AGREGAR UN NUEVO INSTRUMENTO:
 * =============================================================================
 * 1. Crear la clase del instrumento (ej: AttitudeIndicator.h/cpp)
 * 2. Agregar #include en FlightHUD.h
 * 3. Agregar variable miembro en FlightHUD.h privado:
 *    AttitudeIndicator attitudeIndicator_;
 * 4. En init(): Inicializar el instrumento (ver ejemplo del altímetro)
 * 5. En update(): Pasar datos si el instrumento los necesita
 * 6. En render(): Llamar a render() del instrumento
 * 7. En setupInstrumentLayout(): Configurar posición y tamaño
 */

#include "FlightHUD.h"
#include <iostream>

namespace hud
{

    // ============================================================================
    // CONSTRUCTOR
    // ============================================================================

    FlightHUD::FlightHUD() : altimeter_(nullptr), speedIndicator_(nullptr), verticalSpeedIndicator_(nullptr),
                             waypointIndicator_(nullptr), bankAngleIndicator_(nullptr), pitchLadder_(nullptr),
                             screenWidth_(1280), screenHeight_(720)
    {
        // Crear el renderer 2D compartido
        renderer2D_ = std::make_unique<gfx::Renderer2D>();

        // Esquema cromático base (se puede expandir con configuraciones de usuario).
        hudColor_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // Verde HUD
        warningColor_ = glm::vec4(1.0f, 0.85f, 0.2f, 0.95f);
        dangerColor_ = glm::vec4(1.0f, 0.2f, 0.2f, 0.95f);

        // CREAR E INICIALIZAR INSTRUMENTOS
        // Los instrumentos se crean como unique_ptr y se almacenan en el vector
        // También guardamos referencias raw para acceso directo

        // Altimeter
        auto altimeter = std::make_unique<Altimeter>();
        altimeter_ = altimeter.get();
        instruments_.push_back(std::move(altimeter));

        // SpeedIndicator
        auto speedIndicator = std::make_unique<SpeedIndicator>();
        speedIndicator_ = speedIndicator.get();
        instruments_.push_back(std::move(speedIndicator));

        // VerticalSpeedIndicator
        auto verticalSpeedIndicator = std::make_unique<VerticalSpeedIndicator>();
        verticalSpeedIndicator_ = verticalSpeedIndicator.get();
        instruments_.push_back(std::move(verticalSpeedIndicator));

        // WaypointIndicator (HSI)
        auto waypointIndicator = std::make_unique<WaypointIndicator>();
        waypointIndicator_ = waypointIndicator.get();
        instruments_.push_back(std::move(waypointIndicator));

        // BankAngleIndicator
        auto bankAngleIndicator = std::make_unique<BankAngleIndicator>();
        bankAngleIndicator_ = bankAngleIndicator.get();
        instruments_.push_back(std::move(bankAngleIndicator));

        // PitchLadder
        auto pitchLadder = std::make_unique<PitchLadder>();
        pitchLadder_ = pitchLadder.get();
        instruments_.push_back(std::move(pitchLadder));
    }

    // ============================================================================
    // INICIALIZACIÓN DE INSTRUMENTOS
    // ============================================================================

    /**
     * @brief Inicializa el sistema de HUD y todos sus instrumentos
     * @param screenWidth Ancho de la pantalla en píxeles
     * @param screenHeight Alto de la pantalla en píxeles
     *
     * Este método inicializa:
     * 1. El sistema de renderizado 2D compartido
     * 2. El layout de TODOS los instrumentos
     * 3. Cualquier configuración inicial necesaria
     */
    void FlightHUD::init(int screenWidth, int screenHeight)
    {
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        // Inicializar el renderer 2D compartido (proyección ortográfica HUD).
        renderer2D_->init(screenWidth, screenHeight);

        // Configurar layout de todos los instrumentos
        setupInstrumentLayout();

        // Log de inicialización
        std::cout << "Flight HUD initialized: " << screenWidth << "x" << screenHeight << std::endl;
        std::cout << "  - Altimeter: OK" << std::endl;
        std::cout << "  - SpeedIndicator: OK" << std::endl;
        std::cout << "  - VerticalSpeedIndicator: OK" << std::endl;
        std::cout << "  - WaypointIndicator (HSI): OK" << std::endl;
        std::cout << "  - BankAngleIndicator: OK" << std::endl;
        std::cout << "  - PitchLadder: OK" << std::endl;
    }

    /**
     * @brief Ajusta el HUD cuando cambia el tamaño de la ventana
     */
    void FlightHUD::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        renderer2D_->setScreenSize(width, height);

        // Recalcular layout de todos los instrumentos
        setupInstrumentLayout();
    }

    // ============================================================================
    // ACTUALIZACIÓN Y RENDERIZADO DE INSTRUMENTOS
    // ============================================================================

    /**
     * @brief Actualiza los datos de vuelo para todos los instrumentos
     * @param flightData Datos actuales del vuelo (altitud, velocidad, actitud, etc.)
     *
     * Este método copia los datos de vuelo que luego serán pasados a cada
     * instrumento durante el render.
     */
    void FlightHUD::update(const flight::FlightData &flightData)
    {
        currentFlightData_ = flightData;
    }

    /**
     * @brief Renderiza todos los instrumentos del HUD como overlay 2D
     *
     * Utiliza el patrón polimórfico para renderizar todos los instrumentos
     * de forma uniforme mediante el vector de Instrument*.
     *
     * Proceso:
     * 1. Configurar estado OpenGL (blending, depth test)
     * 2. Renderizar cada instrumento habilitado en orden
     * 3. Restaurar estado OpenGL
     */
    void FlightHUD::render()
    {
        // Configurar estado OpenGL para overlay 2D
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST); // HUD siempre visible encima del 3D

        // Comenzar batch de renderizado 2D
        renderer2D_->begin();

        // ========================================================================
        // RENDERIZAR TODOS LOS INSTRUMENTOS POLIMÓRFICAMENTE
        // ========================================================================

        for (const auto &instrument : instruments_)
        {
            if (instrument && instrument->isEnabled())
            {
                instrument->render(*renderer2D_, currentFlightData_);
            }
        }

        // Finalizar batch
        renderer2D_->end();

        // Restaurar estado OpenGL para renderizado 3D
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    // ============================================================================
    // CONFIGURACIÓN DE LAYOUTS
    // ============================================================================

    /**
     * @brief Cambia el layout del HUD
     * @param layoutName Nombre del layout ("classic", "modern", "minimal")
     *
     * Layouts disponibles:
     * - "classic": Layout tradicional de aviación (instrumentos grandes)
     * - "modern": Layout compacto y eficiente
     * - "minimal": Solo información esencial
     */
    void FlightHUD::setLayout(const std::string &layoutName)
    {
        // Reconfigurar todos los instrumentos según el layout seleccionado
        setupInstrumentLayout();
    }

    /**
     * @brief Configura la posición y tamaño de TODOS los instrumentos
     */
    void FlightHUD::setupInstrumentLayout()
    {
        float centerX = screenWidth_ * 0.5f; // Para futuros instrumentos centrados
        float centerY = screenHeight_ * 0.5f;

        // ------------------------------------------------------------------------
        // SPEED INDICATOR (Velocidad) - IZQUIERDA
        // ------------------------------------------------------------------------
        {
            const float WIDTH = 120.0f;
            const float HEIGHT = 450.0f;
            const float MARGIN_LEFT = 30.0f;

            float posX = MARGIN_LEFT;
            float posY = centerY - HEIGHT * 0.5f;

            speedIndicator_->setPosition(glm::vec2(posX, posY));
            speedIndicator_->setSize(glm::vec2(WIDTH, HEIGHT));
            speedIndicator_->setColor(hudColor_);
            speedIndicator_->setEnabled(true);
        }

        // ------------------------------------------------------------------------
        // ALTIMETER (Altímetro) - DERECHA
        // ------------------------------------------------------------------------
        {
            const float WIDTH = 120.0f;
            const float HEIGHT = 450.0f;
            const float MARGIN_RIGHT = 30.0f;

            float posX = screenWidth_ - WIDTH - MARGIN_RIGHT;
            float posY = centerY - HEIGHT * 0.5f;

            altimeter_->setPosition(glm::vec2(posX, posY));
            altimeter_->setSize(glm::vec2(WIDTH, HEIGHT));
            altimeter_->setColor(hudColor_);
            altimeter_->setEnabled(true);
        }

        // ------------------------------------------------------------------------
        // VERTICAL SPEED INDICATOR (VSI) - ENTRE CENTRO Y ALTÍMETRO
        // ------------------------------------------------------------------------
        {
            const float WIDTH = 80.0f;            // Más estrecho que los tapes principales
            const float HEIGHT = 225.0f;          // 50% de altura de los tapes principales
            const float GAP_TO_ALTIMETER = 20.0f; // Separación mínima recomendada
            const float GAP_TO_FPV = 12.0f;       // Separación mínima respecto al eje central

            // Recalcular posición del altímetro (coincidir con bloque anterior)
            const float ALT_WIDTH = 120.0f;
            const float ALT_HEIGHT = 450.0f;
            (void)ALT_HEIGHT;
            const float ALT_MARGIN_RIGHT = 30.0f;
            float altPosX = screenWidth_ - ALT_WIDTH - ALT_MARGIN_RIGHT;

            // Posición ideal: a la izquierda del altímetro, separado fijo
            float desiredRight = altPosX - GAP_TO_ALTIMETER; // borde derecho del VSI
            float posX = desiredRight - WIDTH;               // borde izquierdo del VSI

            // Asegurar separación mínima desde el eje central (FPV)
            float minPosX = centerX + GAP_TO_FPV;
            if (posX < minPosX)
                posX = minPosX;

            float posY = centerY - HEIGHT * 0.5f;

            verticalSpeedIndicator_->setPosition(glm::vec2(posX, posY));
            verticalSpeedIndicator_->setSize(glm::vec2(WIDTH, HEIGHT));
            verticalSpeedIndicator_->setColor(hudColor_);
            verticalSpeedIndicator_->setEnabled(true);
        }

        // ------------------------------------------------------------------------
        // WAYPOINT INDICATOR (Navegación) - CENTRO SUPERIOR
        // ------------------------------------------------------------------------
        {
            const float PANEL_WIDTH = 159.0f; // Bounding box actual de la rosa
            const float PANEL_HEIGHT = 134.0f;
            const float COMPASS_CENTER_OFFSET_X = 92.0f; // 12 + 25 + 55
            const float COMPASS_CENTER_OFFSET_Y = 67.0f; // 12 + 55
            const float ROSE_RADIUS = 55.0f;
            const float TOP_MARGIN = 30.0f;

            float desiredCenterX = centerX;
            float desiredCenterY = TOP_MARGIN + ROSE_RADIUS;

            float posX = desiredCenterX - COMPASS_CENTER_OFFSET_X;
            float posY = desiredCenterY - COMPASS_CENTER_OFFSET_Y;

            waypointIndicator_->setPosition(glm::vec2(posX, posY));
            waypointIndicator_->setSize(glm::vec2(PANEL_WIDTH, PANEL_HEIGHT));
            waypointIndicator_->setColor(hudColor_);
        }

        // ------------------------------------------------------------------------
        // PITCH LADDER (Escalera de Cabeceo) - CENTRO
        // ------------------------------------------------------------------------
        {
            // PitchLadder ocupa toda la pantalla (coordenadas relativas al centro)
            // Le pasamos las dimensiones completas para que pueda calcular conversiones NDC->píxeles
            pitchLadder_->setPosition(glm::vec2(0.0f, 0.0f));
            pitchLadder_->setSize(glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)));
            pitchLadder_->setColor(hudColor_);
            pitchLadder_->setEnabled(true);
        }

        // ------------------------------------------------------------------------
        // BANK ANGLE INDICATOR (Indicador de Alabeo) - ABAJO CENTRO
        // ------------------------------------------------------------------------
        {
            // BankAngleIndicator también usa toda la pantalla para conversiones NDC
            // Se dibuja en la parte inferior (NDC_CENTER_Y = -0.85)
            bankAngleIndicator_->setPosition(glm::vec2(0.0f, 0.0f));
            bankAngleIndicator_->setSize(glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)));
            bankAngleIndicator_->setColor(hudColor_);
            bankAngleIndicator_->setEnabled(true);
        }
    }

} // namespace hud
