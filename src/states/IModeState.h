#pragma once

namespace core
{
    struct AppContext;
}

namespace states
{

    class IModeState
    {
    public:
        virtual ~IModeState() = default;

        virtual void onEnter(core::AppContext &)
        {
        }
        virtual void onExit(core::AppContext &)
        {
        }

        virtual void handleInput(core::AppContext &context) = 0;
        virtual void update(core::AppContext &context) = 0;
        virtual void render(core::AppContext &context) = 0;
    };

} // namespace states
