#pragma once

#include "interface/script/script.h"
#include <wasmtime.hh>

namespace Arieo
{
    /**
     * @brief Wasmtime-based script context implementation
     */
    class WasmtimeContext final
        : public Interface::Script::IContext
    {
    public:
        WasmtimeContext(wasmtime::Engine& engine)
            : m_store(engine)
        {
            // Configure WASI and store it within our `wasmtime_store_t`
            wasmtime::WasiConfig wasi;
            wasi.inherit_argv();
            wasi.inherit_env();
            wasi.inherit_stdin();
            wasi.inherit_stdout();
            wasi.inherit_stderr();
            m_store.context().set_wasi(std::move(wasi)).unwrap();
        }

        void addHostFunction(
            const std::string& module_name,
            const std::string& function_name,
            const std::function<void()>& function
        ) override;
    private:
        friend class WasmtimeEngine;
        friend class WasmtimeInstance;
        wasmtime::Store m_store;
        std::vector<wasmtime::Extern> m_host_externs;
    };
}




