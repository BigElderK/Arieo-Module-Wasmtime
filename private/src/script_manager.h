#pragma once

#include "base/prerequisites.h"
#include "core/core.h"
#include <wasmtime.hh>
#include <unordered_map>
#include <memory>

#include "interface/script/script.h"
#include "interface/main/main_module.h"

namespace Arieo
{
    /**
     * @brief Wasmtime-based scripting engine implementation
     */
    class ScriptManager final
        : public Interface::Main::ITickable
    {
    public:
        void onInitialize() override;
        void onTick() override;
        void onDeinitialize() override;
    private:
    };
}




