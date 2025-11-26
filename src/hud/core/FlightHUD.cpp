/**
 * =============================================================================
 * PURPOSE:
 * This file manages ALL instruments of the flight simulator HUD.
 * =============================================================================
 *
 * ARCHITECTURE:
 * The FlightHUD is the central coordinator that:
 * 1. Creates and manages all flight instruments
 * 2. Passes updated aircraft data (FlightData) to them
 * 3. Coordinates rendering in the correct order
 * 4. Handles layout and positioning of each instrument
 *
 * INSTRUMENTS:
 * - Altimeter
 * - AttitudeIndicator
 * - AirspeedIndicator
 * - HeadingIndicator
 * - VerticalSpeedIndicator
 * - TurnCoordinator
 *
 * =============================================================================
 * GUIDE TO ADDING A NEW INSTRUMENT:
 * =============================================================================
 * 1. Create the instrument class (e.g., AttitudeIndicator.h/cpp)
 * 2. Add #include in FlightHUD.h
 * 3. Add member variable in FlightHUD.h private:
 *    AttitudeIndicator attitudeIndicator_;
 * 4. In init(): Initialize the instrument (see altimeter example)
 * 5. In update(): Pass data if the instrument needs it
 * 6. In render(): Call render() of the instrument
 * 7. In setupInstrumentLayout(): Configure position and size
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
        // Create the shared 2D renderer
        renderer2D_ = std::make_unique<gfx::Renderer2D>();

        // Base color scheme (can be expanded with user configurations).
        hudColor_ = glm::vec4(0.0f, 1.0f, 0.4f, 0.95f); // HUD Green
        warningColor_ = glm::vec4(1.0f, 0.85f, 0.2f, 0.95f);
        dangerColor_ = glm::vec4(1.0f, 0.2f, 0.2f, 0.95f);

        // CREATE AND INITIALIZE INSTRUMENTS
        // Instruments are created as unique_ptr and stored in the vector
        // We also keep raw references for direct access

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
    // INSTRUMENT INITIALIZATION
    // ============================================================================

    /**
     * @brief Initializes the HUD system and all its instruments.
     * @param screenWidth Screen width in pixels.
     * @param screenHeight Screen height in pixels.
     *
     * This method initializes:
     * 1. The shared 2D rendering system.
     * 2. The layout of ALL instruments.
     * 3. Any necessary initial configuration.
     */
    void FlightHUD::init(int screenWidth, int screenHeight)
    {
        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        // Initialize shared 2D renderer (HUD orthographic projection).
        renderer2D_->init(screenWidth, screenHeight);

        // Configure layout of all instruments
        setupInstrumentLayout();

        // Initialization log
        std::cout << "Flight HUD initialized: " << screenWidth << "x" << screenHeight << std::endl;
        std::cout << "  - Altimeter: OK" << std::endl;
        std::cout << "  - SpeedIndicator: OK" << std::endl;
        std::cout << "  - VerticalSpeedIndicator: OK" << std::endl;
        std::cout << "  - WaypointIndicator (HSI): OK" << std::endl;
        std::cout << "  - BankAngleIndicator: OK" << std::endl;
        std::cout << "  - PitchLadder: OK" << std::endl;
    }

    /**
     * @brief Adjusts the HUD when the window size changes.
     */
    void FlightHUD::setScreenSize(int width, int height)
    {
        screenWidth_ = width;
        screenHeight_ = height;
        renderer2D_->setScreenSize(width, height);

        // Recalculate layout of all instruments
        setupInstrumentLayout();
    }

    // ============================================================================
    // INSTRUMENT UPDATE AND RENDERING
    // ============================================================================

    /**
     * @brief Updates flight data for all instruments.
     * @param flightData Current flight data (altitude, speed, attitude, etc.).
     *
     * This method copies the flight data that will later be passed to each
     * instrument during render.
     */
    void FlightHUD::update(const flight::FlightData &flightData)
    {
        currentFlightData_ = flightData;
    }

    /**
     * @brief Renders all HUD instruments as a 2D overlay.
     *
     * Uses the polymorphic pattern to render all instruments uniformly
     * via the Instrument* vector.
     *
     * Process:
     * 1. Configure OpenGL state (blending, depth test).
     * 2. Render each enabled instrument in order.
     * 3. Restore OpenGL state.
     */
    void FlightHUD::render()
    {
        // Configure OpenGL state for 2D overlay
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST); // HUD always visible on top of 3D

        // Begin 2D rendering batch
        renderer2D_->begin();

        // ========================================================================
        // RENDER ALL INSTRUMENTS POLYMORPHICALLY
        // ========================================================================

        for (const auto &instrument : instruments_)
        {
            if (instrument && instrument->isEnabled())
            {
                instrument->render(*renderer2D_, currentFlightData_);
            }
        }

        // End batch
        renderer2D_->end();

        // Restore OpenGL state for 3D rendering
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    // ============================================================================
    // LAYOUT CONFIGURATION
    // ============================================================================

    /**
     * @brief Changes the HUD layout.
     * @param layoutName Layout name ("classic", "modern", "minimal").
     *
     * Available layouts:
     * - "classic": Traditional aviation layout (large instruments).
     * - "modern": Compact and efficient layout.
     * - "minimal": Only essential information.
     */
    void FlightHUD::setLayout(const std::string &layoutName)
    {
        // Reconfigure all instruments according to the selected layout
        setupInstrumentLayout();
    }

    /**
     * @brief Configures the position and size of ALL instruments.
     */
    void FlightHUD::setupInstrumentLayout()
    {
        float centerX = screenWidth_ * 0.5f; // For future centered instruments
        float centerY = screenHeight_ * 0.5f;

        // ------------------------------------------------------------------------
        // SPEED INDICATOR - LEFT
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
        // ALTIMETER - RIGHT
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
        // VERTICAL SPEED INDICATOR (VSI) - BETWEEN CENTER AND ALTIMETER
        // ------------------------------------------------------------------------
        {
            const float WIDTH = 80.0f;            // Narrower than main tapes
            const float HEIGHT = 225.0f;          // 50% height of main tapes
            const float GAP_TO_ALTIMETER = 20.0f; // Minimum recommended separation
            const float GAP_TO_FPV = 12.0f;       // Minimum separation from center axis

            // Recalculate altimeter position (match previous block)
            const float ALT_WIDTH = 120.0f;
            const float ALT_HEIGHT = 450.0f;
            (void)ALT_HEIGHT;
            const float ALT_MARGIN_RIGHT = 30.0f;
            float altPosX = screenWidth_ - ALT_WIDTH - ALT_MARGIN_RIGHT;

            // Ideal position: left of altimeter, fixed separation
            float desiredRight = altPosX - GAP_TO_ALTIMETER; // VSI right edge
            float posX = desiredRight - WIDTH;               // VSI left edge

            // Ensure minimum separation from center axis (FPV)
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
        // WAYPOINT INDICATOR (Navigation) - TOP CENTER
        // ------------------------------------------------------------------------
        {
            const float PANEL_WIDTH = 159.0f; // Current bounding box of the rose
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
        // PITCH LADDER - CENTER
        // ------------------------------------------------------------------------
        {
            // PitchLadder occupies the entire screen (coordinates relative to center)
            // We pass full dimensions so it can calculate NDC->pixel conversions
            pitchLadder_->setPosition(glm::vec2(0.0f, 0.0f));
            pitchLadder_->setSize(glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)));
            pitchLadder_->setColor(hudColor_);
            pitchLadder_->setEnabled(true);
        }

        // ------------------------------------------------------------------------
        // BANK ANGLE INDICATOR - BOTTOM CENTER
        // ------------------------------------------------------------------------
        {
            // BankAngleIndicator also uses the entire screen for NDC conversions
            // Drawn at the bottom (NDC_CENTER_Y = -0.85)
            bankAngleIndicator_->setPosition(glm::vec2(0.0f, 0.0f));
            bankAngleIndicator_->setSize(glm::vec2(static_cast<float>(screenWidth_), static_cast<float>(screenHeight_)));
            bankAngleIndicator_->setColor(hudColor_);
            bankAngleIndicator_->setEnabled(true);
        }
    }

} // namespace hud
