#include "base/prerequisites.h"
#include "script_manager.h"
#include "core/core.h"
#include "core/config/config.h"
#include "core/manifest/manifest.h"

#include "engine/wasmtime_engine.h"
#include "interface/sample/sample.h"

namespace Arieo
{
    void ScriptManager::onInitialize()
    {
        Base::Interop::RawRef<Interface::Main::IMainModule> main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
        
        Core::Manifest manifest;
        std::string manifest_rv = main_module->getManifestContext();
        manifest.loadFromString(manifest_rv.getString());
        Core::ConfigNode system_node = manifest.getSystemNode();

        Core::Logger::info("Parsing script info");
        if(system_node["script_entry"].IsDefined() == false)
        {
            Core::Logger::error("Cannot found 'app.host_os.{}.script_entry' node in manifest", Core::SystemUtility::getHostOSName());
            return;
        }

        std::string script_entry = system_node["script_entry"].as<std::string>();

        Core::Logger::info("Parsing startup_script");
        {
            if(script_entry.starts_with("${SCRIPT_DIR}") == false)
            {
                Core::Logger::error("'app.startup_script' must start with ${SCRIPT_DIR}");
                return;
            }
            
            // TODO: Define SCRIPT_DIR in app.manifest.yaml
            Base::StringUtility::replaceAll(script_entry, "${SCRIPT_DIR}", "script");

            auto starup_script_file = main_module->getRootArchive()->aquireFileBuffer(
                Core::SystemUtility::FileSystem::getFormalizedPath(script_entry)
            );

            Base::Interop::RawRef<Interface::Script::IScriptEngine> script_manager = Core::ModuleManager::getInterface<Interface::Script::IScriptEngine>("wasmtime");
            if(script_manager == nullptr)
            {
                Core::Logger::error("Cannot found script engine module: wasmtime");
                return;
            }

            Base::Interop::RawRef<Arieo::Interface::Script::IContext> script_context = script_manager->createContext();
            Base::Interop::RawRef<Interface::Script::IModule> script_module = script_manager->loadModuleFromCompiledBinary(
                starup_script_file->getBuffer(),
                starup_script_file->getBufferSize()
            );
            main_module->getRootArchive()->releaseFileBuffer(starup_script_file);

            // Get the linker pointer from wasmtime engine (requires casting to access private member)
            // WasmtimeEngine* wasmtime_engine = script_manager.castToInstance<WasmtimeEngine>();
            // wasmtime::component::Linker* linker = static_cast<wasmtime::component::Linker*>(wasmtime_engine->getLinker());

            // Load all interface linkers defined in app.manifest.yaml
            {
                if(system_node["linkers"].IsDefined())
                {
                    Core::Logger::info("Loading interface linkers");
                    
                    // Iterate through each linker in the manifest
                    const auto& linkers_node = system_node["linkers"];
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

            Base::Interop::RawRef<Interface::Script::IInstance> script_instance = script_manager->createInstance(
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




