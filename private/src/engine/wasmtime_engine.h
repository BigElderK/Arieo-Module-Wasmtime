#pragma once

#include "base/prerequisites.h"
#include "core/core.h"
#include <wasmtime.hh>
#include <wasmtime/component.hh>
#include <unordered_map>
#include <memory>

#include "interface/script/script.h"
#include "lib/wasmtime_linker/interface_wasmtime_linker.h"
namespace Arieo
{
    /**
     * @brief Wasmtime-based scripting engine implementation
     */
    class WasmtimeEngine
        : public Interface::Script::IScriptEngine
    {
    public:
        // IScriptEngine interface
        void initialize();
        void shutdown();

        void initInterfaceLinkers(const std::filesystem::path& lib_file_path) override;

        Base::Interop<Interface::Script::IContext> createContext() override;
        void destroyContext(Base::Interop<Interface::Script::IContext> context) override; 

        Base::Interop<Interface::Script::IModule> loadModuleFromWatString(const std::string& wat_string) override;
        Base::Interop<Interface::Script::IModule> loadModuleFromCompiledBinary(void* binary_data, size_t data_size) override;
        void unloadModule(Base::Interop<Interface::Script::IModule> module) override;

        Base::Interop<Interface::Script::IInstance> createInstance(Base::Interop<Interface::Script::IContext> context, Base::Interop<Interface::Script::IModule> module) override;
        void destroyInstance(Base::Interop<Interface::Script::IInstance> instance) override;

        // Get the wasmtime linker for interface registration
        void* getLinker() { return m_linker; }

    private:
        wasmtime::Engine* m_engine = nullptr;
        wasmtime::component::Linker* m_linker = nullptr;

        std::unordered_map<std::uint64_t, Lib::WasmtimeLinker::InterfaceExportInfo*> m_interface_export_map;
    };
}