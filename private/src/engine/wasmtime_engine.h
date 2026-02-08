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

        Interface::Script::IContext* createContext() override;
        void destroyContext(Interface::Script::IContext* context) override; 

        Interface::Script::IModule* loadModuleFromWatString(const std::string& wat_string) override;
        Interface::Script::IModule* loadModuleFromCompiledBinary(void* binary_data, size_t data_size) override;
        void unloadModule(Interface::Script::IModule* module) override;

        Interface::Script::IInstance* createInstance(Interface::Script::IContext* context, Interface::Script::IModule* module) override;
        void destroyInstance(Interface::Script::IInstance* instance) override;

        // Get the wasmtime linker for interface registration
        void* getLinker() { return m_linker; }

    private:
        wasmtime::Engine* m_engine = nullptr;
        wasmtime::component::Linker* m_linker = nullptr;

        std::unordered_map<std::uint64_t, Lib::WasmtimeLinker::InterfaceExportInfo*> m_interface_export_map;
    };
}