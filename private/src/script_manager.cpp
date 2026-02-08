#include "base/prerequisites.h"
#include "script_manager.h"
#include "core/core.h"
#include "core/config/config.h"

#include "engine/wasmtime_engine.h"
#include "interface/sample/sample.h"

namespace Arieo
{
    void ScriptManager::onInitialize()
    {
        Interface::Main::IMainModule* main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
        Core::ConfigNode&& config_node = Core::ConfigFile::Load(main_module->getManifestContent());
        if(config_node.IsNull())
        {
            Core::Logger::error("Load manifest file failed in ScriptManager");
            return;
        }

        Core::Logger::info("Parsing script info");
        if(config_node["app"]["script"]["startup_script"].IsDefined() == false)
        {
            Core::Logger::error("Cannot found 'app.script.startup_script' node in manifest");
            return;
        }

        std::string startup_script_path = config_node["app"]["script"]["startup_script"].as<std::string>();
        Core::Logger::info("Parsing startup_script");
        {
            if(startup_script_path.starts_with("${SCRIPT_DIR}") == false)
            {
                Core::Logger::error("'app.startup_script' must start with ${SCRIPT_DIR}");
                return;
            }
            
            // TODO: Define SCRIPT_DIR in app.manifest.yaml
            Base::StringUtility::replaceAll(startup_script_path, "${SCRIPT_DIR}", "script");

            std::tuple<void*, size_t> starup_script_path_buffer = main_module->getRootArchive()->getFileBuffer(
                Core::SystemUtility::FileSystem::getFormalizedPath(startup_script_path)
            );

            Interface::Script::IScriptEngine* script_manager = Core::ModuleManager::getInterface<Interface::Script::IScriptEngine>("wasmtime");
            if(script_manager == nullptr)
            {
                Core::Logger::error("Cannot found script engine module: wasmtime");
                return;
            }

            Arieo::Interface::Script::IContext* script_context = script_manager->createContext();
            Interface::Script::IModule* script_module = script_manager->loadModuleFromCompiledBinary(
                std::get<0>(starup_script_path_buffer),
                std::get<1>(starup_script_path_buffer)
            );

            // Get the linker pointer from wasmtime engine (requires casting to access private member)
            WasmtimeEngine* wasmtime_engine = static_cast<WasmtimeEngine*>(script_manager);
            wasmtime::component::Linker* linker = static_cast<wasmtime::component::Linker*>(wasmtime_engine->getLinker());



            // Load all interface linkers defined in app.manifest.yaml
            {
                if(config_node["app"]["script"]["linkers"].IsDefined())
                {
                    Core::Logger::info("Loading interface linkers");
                                        
                    // Get version checksum (can be used for compatibility checking)
                    std::uint64_t version_checksum = 0; // TODO: Calculate actual version checksum
                    
                    // Iterate through each linker in the manifest
                    const auto& linkers_node = config_node["app"]["script"]["linkers"];
                    for(size_t i = 0; i < linkers_node.size(); ++i)
                    {
                        std::string linker_lib_path = linkers_node[i].as<std::string>();                        
                        Core::Logger::info("Loading interface linker: {}", linker_lib_path);

                        script_manager->initInterfaceLinkers(
                            Core::SystemUtility::FileSystem::getFormalizedPath(linker_lib_path)
                        );
                    }
                }
                else
                {
                    Core::Logger::info("No interface linkers defined in manifest");
                }
            }

            // {
            //     WasmtimeEngine* wasmtime_engine = static_cast<WasmtimeEngine*>(script_manager);
            //     wasmtime::component::LinkerInstance linker = (reinterpret_cast<wasmtime::component::Linker*>(wasmtime_engine->getLinker()))->root();
            //     linkInterfaces(&linker, 0);
            // }

            Interface::Script::IInstance* script_instance = script_manager->createInstance(
                script_context,
                script_module
            );

            script_instance->callFunction(
                script_instance->queryFunction(
                    script_instance->queryInterface("wasi:cli/run@0.2.0"),
                    "run"
                )
            );

            script_manager->destroyInstance(script_instance);
            script_manager->unloadModule(script_module);
            script_manager->destroyContext(script_context);
            
        }
    }

    void ScriptManager::onTick()
    {

    }

    void ScriptManager::onDeinitialize()
    {

    }
}