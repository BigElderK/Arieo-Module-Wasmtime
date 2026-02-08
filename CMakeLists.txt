cmake_minimum_required (VERSION 3.31)

arieo_engine_project(
    arieo_wasmtime_module
    ALIAS arieo::modules::scripting::wasmtime
    PROJECT_TYPE module
    SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/private/src/*/*.cpp
    PACKAGES
        wasmtime
    INTERFACES
        arieo::interface::script
        arieo::interface::main
        arieo::interface::sample
    LIBS
        arieo::base
        arieo::core
        arieo::lib::wasmtime_linker
        wasmtime::wasmtime
)