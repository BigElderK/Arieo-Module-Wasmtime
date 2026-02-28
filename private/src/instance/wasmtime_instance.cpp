#include "base/prerequisites.h"
#include "wasmtime_instance.h"
#include "core/logger/logger.h"
namespace Arieo
{
    void* WasmtimeInstance::queryInterface(const std::string& interface_name)
    {
        // std::optional<wasmtime::component::ExportIndex> export_interface_index = m_instance.get_export_index(
        //     m_store.context(), nullptr, interface_name.c_str());
        wasmtime_component_export_index_t* ret = wasmtime_component_instance_get_export_index(
            m_instance.capi(),
            m_store.context().capi(),
            nullptr,
            interface_name.c_str(),
            interface_name.size()
        );
        if(ret == nullptr)
        {
            Core::Logger::error("Failed to find interface: {}", interface_name);
            return nullptr;
        }
        return ret;
    }

    void* WasmtimeInstance::queryFunction(void* interface, const std::string& function_name)
    {
        // std::optional<wasmtime::component::ExportIndex> export_function_index = m_instance.get_export_index(
        //     m_store.context(), static_cast<const wasmtime_component_export_index_t*>(interface), "run");

        wasmtime_component_export_index_t* function_index = wasmtime_component_instance_get_export_index(
            m_instance.capi(),
            m_store.context().capi(),
            static_cast<const wasmtime_component_export_index_t*>(interface),
            function_name.c_str(),
            function_name.size()
        );
        if(function_index == nullptr)
        {
            Core::Logger::error("Failed to find function: {}", function_name);
            return nullptr;
        }
        return function_index;
    }

    void WasmtimeInstance::callFunction(void* function)
    {
        wasmtime_component_func_t wasmtime_function;
        if(wasmtime_component_instance_get_func(
            m_instance.capi(),
            m_store.context().capi(),
            static_cast<const wasmtime_component_export_index_t*>(function),
            &wasmtime_function
        ) == false)
        {
            Core::Logger::error("Failed to get run function from WASM module");
            return;
        }
        std::vector<wasmtime_component_val_t> results(1);
        wasmtime_error_t *error = wasmtime_component_func_call(
            &wasmtime_function, 
            m_store.context().capi(), 
            nullptr, 0,
            results.data(), 
            results.size()
        );
        
        if (error != nullptr) 
        {
            Core::Logger::error("Error calling run function in WASM module: {}", wasmtime::Error(error).message());
        }
    }

    /*
    void WasmtimeInstance::run(const std::string& function_name)
    {
        Core::Logger::info("Running Wasmtime instance function '{}'", function_name);


        std::optional<wasmtime::component::ExportIndex> export_interface_index = m_instance.get_export_index(
            m_store.context(), nullptr, "wasi:cli/run@0.2.0");
        if(export_interface_index == std::nullopt)
        {
            Core::Logger::error("Failed to find wasi:cli/run@0.2.0 export in WASM module");
            return;
        }

        std::optional<wasmtime::component::ExportIndex> export_function_index = m_instance.get_export_index(
            m_store.context(), &export_interface_index.value(), "run");
        if(export_function_index == std::nullopt)
        {
            Core::Logger::error("Failed to find run export in wasi:cli/run@0.2.0 WASM module");
            return;
        }

        std::optional<wasmtime::component::Func> start_func = m_instance.get_func(m_store.context(), *export_function_index);
        if(start_func == std::nullopt)
        {
            Core::Logger::error("Failed to get run function from WASM module");
            return;
        }

        // `wasi:cli/run` returns a single result (an exit code). Provide a
        // one-element buffer to receive it instead of an empty vector.
        wasmtime::component::Val return_value{0};
        wasmtime::Result<std::monostate> result = (*start_func).call(m_store.context(), {}, {&return_value, 1});

        if(!result)
        {
            Core::Logger::error("Error calling run function in WASM module: " + result.err().message());
            return;
        }
    }
    */
}




