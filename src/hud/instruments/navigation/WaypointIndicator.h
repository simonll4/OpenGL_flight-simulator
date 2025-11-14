#pragma once
#include "../../core/Instrument.h"

namespace hud
{
    /**
     * @class WaypointIndicator
     * @brief Indicador de navegación HSI (Horizontal Situation Indicator)
     *
     * Implementación de un indicador de navegación inspirado en el HSI de Garmin.
     * Proporciona información visual completa para navegación por waypoints mediante:
     *
     * - Rosa de los vientos rotativa con marcas cada 5° y 10°
     * - Etiquetas en los puntos cardinales (N, E, S, W)
     * - Números de rumbo cada 30° (000, 030, 060, etc.)
     * - Flecha magenta indicando dirección al waypoint
     * - Información digital: distancia, ángulo de giro y bearing
     * - Barra de proximidad al waypoint
     * - Indicador vertical de diferencia de altura
     *
     * El HSI combina la funcionalidad de una brújula con un indicador de navegación,
     * permitiendo al piloto visualizar simultáneamente el rumbo actual y la dirección
     * hacia el waypoint objetivo sin necesidad de cálculos mentales.
     */
    class WaypointIndicator : public Instrument
    {
    public:
        WaypointIndicator();

        /**
         * @brief Renderiza el indicador HSI con los datos actuales de vuelo
         * @param renderer Renderizador 2D compartido
         * @param flightData Datos del vuelo (incluye posición, rumbo, waypoint activo)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        /**
         * @struct NavSnapshot
         * @brief Instantánea de datos de navegación procesados
         *
         * Estructura que contiene todos los datos de navegación calculados
         * en un momento dado, facilitando el paso de información entre métodos
         * de renderizado y mejorando la legibilidad del código.
         */
        struct NavSnapshot
        {
            float heading = 0.0f;           ///< Rumbo actual del avión (0-360°)
            float distance = 0.0f;          ///< Distancia al waypoint en metros
            float bearing = 0.0f;           ///< Bearing absoluto al waypoint (0-360°)
            float relativeAngle = 0.0f;     ///< Ángulo relativo al waypoint (-180 a +180°)
            float altitudeDifference = 0.0f;///< Diferencia de altura con el waypoint (metros)
        };

        /**
         * @brief Construye un snapshot con los datos de navegación actuales
         * @param flightData Datos de vuelo del simulador
         * @return NavSnapshot con datos procesados listos para renderizar
         */
        NavSnapshot buildNavSnapshot(const flight::FlightData &flightData) const;

        // Métodos de renderizado por componente
        void drawMainPanel(gfx::Renderer2D &renderer);
        void drawCompassRose(gfx::Renderer2D &renderer, const NavSnapshot &nav);
        void drawInfoPanel(gfx::Renderer2D &renderer, const NavSnapshot &nav);
        void drawVerticalIndicator(gfx::Renderer2D &renderer, const NavSnapshot &nav);
        void drawNoWaypointMessage(gfx::Renderer2D &renderer);
    };

} // namespace hud
