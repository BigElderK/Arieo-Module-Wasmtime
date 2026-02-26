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
        Base::Instance<WasmtimeEngine> wasmtime_engine;
        Base::Instance<ScriptManager> script_manager;

        DllLoader()
        {
            wasmtime_engine.initialize();

            Core::ModuleManager::registerInstance<Interface::Script::IScriptEngine, WasmtimeEngine>(
                "wasmtime", 
                wasmtime_engine
            );

            Base::Interface<Interface::Main::IMainModule> main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->registerTickable(script_manager->queryInterface<Interface::Main::ITickable>());
        }

        ~DllLoader()
        {
            Base::Interface<Interface::Main::IMainModule> main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->unregisterTickable(script_manager->queryInterface<Interface::Main::ITickable>());

            Core::ModuleManager::unregisterInstance<Interface::Script::IScriptEngine, WasmtimeEngine>(
                wasmtime_engine
            );
            wasmtime_engine.shutdown();
        }
    } dll_loader;
}