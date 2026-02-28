#pragma once

#include "interface/script/script.h"
#include <wasmtime.hh>
#include <wasmtime/component.hh>

namespace Arieo
{
    /**
     * @brief Wasmtime-based script module implementation
     */
    class WasmtimeInstance 
        : public Interface::Script::IInstance
    {
    public:
        WasmtimeInstance(wasmtime::component::Instance&& instance, wasmtime::Store& store)
            : m_instance(std::move(instance)), m_store(store)
        {
        };

        void* queryInterface(const std::string& interface_name) override;
        void* queryFunction(void* interface, const std::string& function_name) override;
        void callFunction(void* function) override;
    private:
        wasmtime::component::Instance m_instance;
        wasmtime::Store& m_store;
    };
}




