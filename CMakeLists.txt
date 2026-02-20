cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)
project(ArieoEngine)

arieo_engine_project(
    arieo_wasmtime_module
    PROJECT_TYPE module

    DEPENDENCIES
        ARIEO_PACKAGES
            Arieo-Core
            Arieo-Lib-WasmtimeLinker
            Arieo-Interface-Script
            Arieo-Interface-Sample
        THIRDPARTY_PACKAGES
            wasmtime
        INTERFACES
            Arieo-Interface-Script::arieo_script_interface
            Arieo-Interface-Main::arieo_main_interface
            Arieo-Interface-Sample::arieo_sample_interface
        PRIVATE_LIBS
            Arieo-Core::arieo_core
            Arieo-Lib-WasmtimeLinker::arieo_wasmtime_linker_lib
            wasmtime::wasmtime

    SOURCES
        CXX_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
)
