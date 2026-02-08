#include "base/prerequisites.h"
#include "wasmtime_context.h"
#include "../instance/wasmtime_instance.h"
#include "../module/wasmtime_module.h"
#include "core/logger/logger.h"

namespace Arieo
{
    void WasmtimeContext::addHostFunction(
        const std::string& module_name,
        const std::string& function_name,
        const std::function<void()>& function
    )
    {
        Core::Logger::info("Adding host function '{}' in module '{}' to Wasmtime context", function_name, module_name);
        wasmtime::Func host_function = wasmtime::Func::wrap(
            m_store,
            function
        );

        m_host_externs.push_back(std::move(host_function));
    }
}