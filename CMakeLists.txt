cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_BUILD_ENV_PACKAGES_OUTPUT_DIR}/cmake/build_environment.cmake)

ARIEO_PACKAGE(
    Arieo-Module-Wasmtime
    CATEGORY 02_engine
    DEPENDS
        https://github.com/BigElderK/Arieo-Core.git@main
        https://github.com/BigElderK/Arieo-Lib-WasmtimeLinker.git@main
        https://github.com/BigElderK/Arieo-Interface-Script.git@main
        https://github.com/BigElderK/Arieo-Interface-Sample.git@main
)

ARIEO_ENGINE_PROJECT(
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
        PRIVATE_LIBS
            Arieo-Interface-Script::arieo_script_interface
            Arieo-Interface-Main::arieo_main_interface
            Arieo-Interface-Sample::arieo_sample_interface
            Arieo-Core::arieo_core
            Arieo-Lib-WasmtimeLinker::arieo_wasmtime_linker_lib
            wasmtime::wasmtime

    SOURCES
        CXX_SOURCES
            ${CMAKE_CURRENT_LIST_DIR}/private/src/*.cpp
            ${CMAKE_CURRENT_LIST_DIR}/private/src/*/*.cpp
)
