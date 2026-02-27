#include "base/prerequisites.h"
#include "core/logger/logger.h"

#include "wasmtime_engine.h"
#include "../context/wasmtime_context.h"
#include "../module/wasmtime_module.h"
#include "../instance/wasmtime_instance.h"

#include "lib/wasmtime_linker/interface_wasmtime_linker.h"

#include "interface/sample/sample.h"

#include <wasmtime.hh>
#include <wasmtime/component.hh>

#include <fstream>
#include <algorithm>
#include <vector>

namespace Arieo
{
    void test_function()
    {
        Core::Logger::info("WasmtimeEngine test_function called");
    }

    void WasmtimeEngine::initialize()
    {
        // Create engine configuration
        wasmtime::Config config;
        config.debug_info(true); // Equivalent to: -D debug-info=y
        config.cranelift_opt_level(wasmtime::OptLevel::None); // Equivalent to: -O opt-level=0
        config.wasm_component_model(true); // Enable component model support
        
        // Enable native unwinding for debugger integration
        // On Windows, this enables debugging without needing Linux-specific profiling
        config.native_unwind_info(true); // Enables native debugger support (LLDB/GDB)

        // config.native_unwind_info(true); // Enable native stack unwinding for debuggers
        // config.cranelift_debug_verifier(false); // Disable verifier that may interfere with debugging
        // config.consume_fuel(false); // Disable fuel consumption
        // config.epoch_interruption(false); // Disable epoch interruption
        // config.macos_use_mach_ports(false); // Use standard GDB JIT interface on all platforms
        

        // Create engine and store with this config
        m_engine = Base::newT<wasmtime::Engine>(std::move(config));

        m_linker = Base::newT<wasmtime::component::Linker>(*m_engine);
        m_linker->add_wasip2().unwrap();
        Core::Logger::info("Wasmtime scripting engine initialized");

        // initial default interface linkers (if any)
        {
            // Component Model: Define host interface implementation
            // For the WIT interface: interface host { log: func(msg: string); }
            auto host_instance = m_linker->root().add_instance("arieo:application/host").unwrap();
            host_instance.add_func(
                "log",
                [](wasmtime::Store::Context store_ctx, 
                const wasmtime::component::FuncType& func_type,
                wasmtime::Span<wasmtime::component::Val> args,
                wasmtime::Span<wasmtime::component::Val> results) -> wasmtime::Result<std::monostate> {
                    // Extract string from args[0]
                    if (args.size() > 0 && args[0].is_string() == true) {
                        auto message = args[0].get_string();
                        Core::Logger::info("[WASM Guest] {}", message);
                    }
                    test_function();
                    return wasmtime::Result<std::monostate>(std::monostate{});
                }
            ).unwrap();

            // Define module-manager interface implementation
            // For the WIT interface: interface module-manager { create-instance: func(ptr: s32, len: s32) -> s64; }
            auto module_manager_instance = m_linker->root().add_instance("arieo:module/module-manager").unwrap();
            module_manager_instance.add_func(
                "get-interface",
                [this](wasmtime::Store::Context store_ctx, 
                const wasmtime::component::FuncType& func_type,
                wasmtime::Span<wasmtime::component::Val> args,
                wasmtime::Span<wasmtime::component::Val> results) -> wasmtime::Result<std::monostate> 
                {
                    // Extract pointer and length from args
                    if (args.size() >= 3) 
                    {
                        uint64_t interface_id = args[0].get_u64();
                        uint64_t interface_checksum = args[1].get_u64();
                        std::string_view instance_name = args[2].get_string(); // unused for now
                        
                        Core::Logger::info("[Host] GetInstance called with interface_id {}, interface_checksum {}, interface_name {}", interface_id, interface_checksum, instance_name);
                        
                        std::uint64_t instance_handle = 0;

                        // search for interface in map
                        auto found_export_interface_info_iter = m_interface_export_map.find(interface_id);
                        if(found_export_interface_info_iter == m_interface_export_map.end())
                        {
                            Core::Logger::error("Interface ID {} not found in registered interface map", interface_id);
                            // Return null instance handle
                            if (results.size() > 0) {
                                results[0] = wasmtime::component::Val(0);
                            }
                            return wasmtime::Result<std::monostate>(std::monostate{});
                        }

                        // check the checksum
                        if(found_export_interface_info_iter->second->m_interface_checksum != interface_checksum)
                        {
                            Core::Logger::error("Interface ID {} checksum mismatch: expected {}, got {}", 
                                interface_id,
                                found_export_interface_info_iter->second->m_interface_checksum,
                                interface_checksum);
                            // Return null instance handle
                            if (results.size() > 0) {

                                results[0] = wasmtime::component::Val(0);
                            }
                            return wasmtime::Result<std::monostate>(std::monostate{});
                        }

                        // Call the interface create function callback to create the instance
                        Core::Logger::trace("Creating interface instance for ID {}", interface_id);

                        instance_handle = reinterpret_cast<uint64_t>(::Core::ModuleManager::getInterfaceRaw(
                            found_export_interface_info_iter->second->m_interface_type_hash,
                            std::string(instance_name)
                        ));
                        
                        // Return instance handle
                        if (results.size() > 0) {
                            results[0] = wasmtime::component::Val(instance_handle);
                        }
                    }

                    return wasmtime::Result<std::monostate>(std::monostate{});
                }
            ).unwrap();
        }
    }

    void WasmtimeEngine::initInterfaceLinkers(const std::filesystem::path& linker_lib_path)
    {
        // const char* version = wasmtime_version_str();

        // Load the dynamic library
        Core::SystemUtility::Lib::LIBTYPE lib_handle = Core::SystemUtility::Lib::loadLibrary(
            linker_lib_path
        );
        
        if(lib_handle == nullptr)
        {
            Core::Logger::error("Failed to load interface linker: {} - {}", 
                linker_lib_path, 
                Core::SystemUtility::Lib::getLastError());
            return;
        }
        
        // Get the linkInterfaces function pointer
        Lib::WasmtimeLinker::DLLExportLinkInterfacesFn get_wasm_time_linker_info_fn = reinterpret_cast<Lib::WasmtimeLinker::DLLExportLinkInterfacesFn>(
                Core::SystemUtility::Lib::getProcAddress(lib_handle, "getWasmtimeLinkerInfo")
        );
        
        if(get_wasm_time_linker_info_fn == nullptr)
        {
            Core::Logger::error("Failed to find 'linkInterfaces' function in: {} - {}", 
                linker_lib_path,
                Core::SystemUtility::Lib::getLastError());
            return;
        }
        
        // Call the linkInterfaces function to register interfaces with wasmtime
        Lib::WasmtimeLinker::LinkerExportInfo* linker_export_info = get_wasm_time_linker_info_fn(
            0);

        // Foreach interface in the linker export info, register it with the wasmtime linker
        for(size_t j = 0; j < linker_export_info->m_interface_count; ++j)
        {
            Lib::WasmtimeLinker::InterfaceExportInfo* interface_export_info = &linker_export_info->m_interface_array[j];
            auto instance = m_linker->root().add_instance(interface_export_info->m_interface_name).unwrap();

            m_interface_export_map.emplace(
                interface_export_info->m_interaface_id,
                interface_export_info
            );

            // Register interface create interface functions
            Core::Logger::info("Registering interface: {} (ID: {}, Checksum: {}) with {} functions",
                interface_export_info->m_interface_name,
                interface_export_info->m_interaface_id,
                interface_export_info->m_interface_checksum,
                interface_export_info->m_member_function_count);

            // Register each function in the interface
            for(size_t k = 0; k < interface_export_info->m_member_function_count; ++k)
            {
                Lib::WasmtimeLinker::InterfaceFunctionExportInfo& function_export_info = interface_export_info->m_member_function_array[k];
                instance.add_func(
                    function_export_info.m_function_name,
                    function_export_info.m_host_callback
                ).unwrap();
            }
        }
    }

    void WasmtimeEngine::shutdown()
    {
        if(m_linker != nullptr)
        {
            Base::deleteT(m_linker);
            m_linker = nullptr;
        }

        if (m_engine != nullptr)
        {
            Base::deleteT(m_engine);
            m_engine = nullptr;
            Core::Logger::info("Wasmtime scripting engine shut down");
        }
    }

    Base::Interop<Interface::Script::IContext> WasmtimeEngine::createContext()
    {
        Core::Logger::info("Creating Wasmtime script context");
        return Base::newT<WasmtimeContext>(*m_engine);
    }

    void WasmtimeEngine::destroyContext(Base::Interop<Interface::Script::IContext> context)
    {
        WasmtimeContext* wasmtime_context = context.castTo<WasmtimeContext>();
        Base::deleteT(wasmtime_context);
        Core::Logger::info("Destroying Wasmtime script context");
    }

    Base::Interop<Interface::Script::IModule> WasmtimeEngine::loadModuleFromWatString(const std::string& wat_string)
    {   
        return nullptr;
    }

    Base::Interop<Interface::Script::IModule> WasmtimeEngine::loadModuleFromCompiledBinary(void* binary_data, size_t data_size)
    {
        if (!binary_data || data_size == 0)
        {
            Core::Logger::error("Invalid binary data or size for WASM module");
            return nullptr;
        }
        
        wasmtime::Result<wasmtime::component::Component> compile_result = wasmtime::component::Component::compile(
            *m_engine,
            wasmtime::Span<uint8_t>(static_cast<uint8_t*>(binary_data), data_size)
        ).unwrap();

        if(!compile_result)
        {
            Core::Logger::error("Failed to compile WASM module from binary data: " + compile_result.err().message());
            return nullptr;
        }
        return Base::newT<WasmtimeModule>(compile_result.unwrap());
    }

    void WasmtimeEngine::unloadModule(Base::Interop<Interface::Script::IModule> module)
    {
        Core::Logger::info("Unloading Wasmtime script module");
        WasmtimeModule* wasmtime_module = module.castTo<WasmtimeModule>();
        Base::deleteT(wasmtime_module);
    }

    Base::Interop<Interface::Script::IInstance> WasmtimeEngine::createInstance(Base::Interop<Interface::Script::IContext> context, Base::Interop<Interface::Script::IModule> module)
    {
        Core::Logger::info("Creating Wasmtime script instance");
        WasmtimeContext* wasmtime_context = context.castTo<WasmtimeContext>();
        WasmtimeModule* wasmtime_module = module.castTo<WasmtimeModule>();

        // Instantiate the component and verify it exports the guest interface
        wasmtime::component::Instance instance = m_linker->instantiate(wasmtime_context->m_store.context(), wasmtime_module->m_component).unwrap();

        // Create the instance wrapper
        WasmtimeInstance* wasmtime_instance = Base::newT<WasmtimeInstance>(std::move(instance), wasmtime_context->m_store);

        // Query the guest interface and function index from the instance wrapper
        void* iface = wasmtime_instance->queryInterface("arieo:application/guest");
        if (iface == nullptr)
        {
            Core::Logger::error("Failed to query interface 'arieo:application/guest' on instance");
            Base::deleteT(wasmtime_instance);
            return nullptr;
        }

        void* func = wasmtime_instance->queryFunction(iface, "greeting");
        if (func == nullptr)
        {
            Core::Logger::error("Failed to query function 'greeting' on interface 'arieo:greeter/guest'");
            Base::deleteT(wasmtime_instance);
            return nullptr;
        }

        // Call the greeting function (no arguments, one result)
        Core::Logger::info("Calling function 'greeting' on interface 'arieo:greeter/guest'");
        wasmtime_instance->callFunction(func);
        Core::Logger::info("Called function 'greeting' on interface 'arieo:greeter/guest'");

        std::this_thread::sleep_for(std::chrono::milliseconds(100000)); // Give some time for async logging to complete

        return wasmtime_instance;
    }

    void WasmtimeEngine::destroyInstance(Base::Interop<Interface::Script::IInstance> instance)
    {
        Core::Logger::info("Destroying Wasmtime script instance");
        WasmtimeInstance* wasmtime_instance = instance.castTo<WasmtimeInstance>();
        Base::deleteT(wasmtime_instance);
    }
}