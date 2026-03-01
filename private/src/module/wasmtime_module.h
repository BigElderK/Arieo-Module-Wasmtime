#pragma once

#include "interface/script/script.h"
#include <wasmtime.hh>
#include <wasmtime/component.hh>
namespace Arieo
{
    /**
     * @brief Wasmtime-based script module implementation
     */
    class WasmtimeModule final : public Interface::Script::IModule
    {
    public:
        WasmtimeModule(wasmtime::component::Component&& component)
            : m_component(std::move(component))
        {
        };
    private:
        friend class WasmtimeEngine;
        friend class WasmtimeContext;
        wasmtime::component::Component m_component;
    };
}




