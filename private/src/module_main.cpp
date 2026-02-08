#include "base/prerequisites.h"
#include "core/core.h"
#include "engine/wasmtime_engine.h"
#include "interface/main/main_module.h"
#include "script_manager.h"
using namespace Arieo;

GENERATOR_MODULE_ENTRY_FUN()
ARIEO_DLLEXPORT void ModuleMain()
{
    Core::Logger::setDefaultLogger("wasmtime");

    static struct DllLoader
    {
        WasmtimeEngine wasmtime_engine;
        ScriptManager script_manager;

        DllLoader()
        {
            wasmtime_engine.initialize();

            Core::ModuleManager::registerInterface<Interface::Script::IScriptEngine>(
                "wasmtime", 
                &wasmtime_engine
            );

            Interface::Main::IMainModule* main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->registerTickable(&script_manager);
        }

        ~DllLoader()
        {
            Interface::Main::IMainModule* main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->unregisterTickable(&script_manager);

            Core::ModuleManager::unregisterInterface<Interface::Script::IScriptEngine>(
                &wasmtime_engine
            );
            wasmtime_engine.shutdown();
        }
    } dll_loader;
}