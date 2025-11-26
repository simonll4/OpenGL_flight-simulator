/**
 * @file IModeState.h
 * @brief Interface for application state behaviors.
 */

#pragma once

namespace core
{
    struct AppContext;
}

namespace states
{

    /**
     * @class IModeState
     * @brief Interface for application states (modes).
     *
     * Defines the lifecycle methods for different application modes
     * (e.g., Menu, Planning, Flight).
     */
    class IModeState
    {
    public:
        virtual ~IModeState() = default;

        /**
         * @brief Called when entering the state.
         * @param context Application context.
         */
        virtual void onEnter(core::AppContext &)
        {
        }

        /**
         * @brief Called when exiting the state.
         * @param context Application context.
         */
        virtual void onExit(core::AppContext &)
        {
        }

        /**
         * @brief Handles user input.
         * @param context Application context.
         */
        virtual void handleInput(core::AppContext &context) = 0;

        /**
         * @brief Updates the state logic.
         * @param context Application context.
         */
        virtual void update(core::AppContext &context) = 0;

        /**
         * @brief Renders the state.
         * @param context Application context.
         */
        virtual void render(core::AppContext &context) = 0;
    };

} // namespace states
