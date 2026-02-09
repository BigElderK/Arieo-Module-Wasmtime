cmake_minimum_required (VERSION 3.31)

include($ENV{ARIEO_BUILDENV_PACKAGE_INSTALL_FOLDER}/cmake/build_environment.cmake)
project(ArieoEngine)

arieo_engine_project(
    arieo_wasmtime_module
    PROJECT_TYPE module

    PACKAGES
        wasmtime
        Arieo-Interface-Script
        Arieo-Interface-Main
        Arieo-Interface-Sample
        Arieo-Lib-WasmtimeLinker
        Arieo-Core
    INTERFACES
        Arieo-Interface-Script::arieo_script_interface
        Arieo-Interface-Main::arieo_main_interface
        Arieo-Interface-Sample::arieo_sample_interface
    PRIVATE_LIBS
        Arieo-Core::arieo_core
        Arieo-Lib-WasmtimeLinker::arieo_wasmtime_linker_lib
        wasmtime::wasmtime

    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
)