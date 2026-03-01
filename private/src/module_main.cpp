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
        WasmtimeEngine wasmtime_engine_instance;
        Base::Interop::SharedRef<Interface::Script::IScriptEngine> wasmtime_engine = Base::Interop::makePersistentShared<Interface::Script::IScriptEngine>(wasmtime_engine_instance);
        ScriptManager script_manager_instance;
        Base::Interop::SharedRef<Interface::Main::ITickable> script_manager = Base::Interop::makePersistentShared<Interface::Main::ITickable>(script_manager_instance);

        DllLoader()
        {
            wasmtime_engine_instance.initialize();

            Core::ModuleManager::registerInterface<Interface::Script::IScriptEngine>(
                "wasmtime",
                wasmtime_engine
            );

            Base::Interop::SharedRef<Interface::Main::IMainModule> main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->registerTickable(script_manager);
        }

        ~DllLoader()
        {
            Base::Interop::SharedRef<Interface::Main::IMainModule> main_module = Core::ModuleManager::getInterface<Interface::Main::IMainModule>();
            main_module->unregisterTickable(script_manager);

            Core::ModuleManager::unregisterInterface<Interface::Script::IScriptEngine>(
                wasmtime_engine
            );
            wasmtime_engine_instance.shutdown();
        }
    } dll_loader;
}




