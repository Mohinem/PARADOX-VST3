cmake_minimum_required(VERSION 3.22)
project(PARADOX VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)
option(PARADOX_BUILD_PLUGINS "Build both JUCE VST3 plugins" ON)
add_library(paradox_engine STATIC
    Source/Law/Law.cpp Source/Events/Events.cpp Source/Engine/Engine.cpp
    Source/Rendering/Renderers.cpp Source/Persistence/State.cpp)
set_target_properties(paradox_engine PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(paradox_engine PUBLIC Source)
if(BUILD_TESTING)
    add_executable(paradox_engine_tests Tests/EngineTests.cpp)
    target_link_libraries(paradox_engine_tests PRIVATE paradox_engine)
    add_test(NAME shared_engine COMMAND paradox_engine_tests)
endif()
if(PARADOX_BUILD_PLUGINS)
    set(PARADOX_JUCE_PATH "" CACHE PATH "Existing JUCE 8.0.6 checkout (optional)")
    if(PARADOX_JUCE_PATH)
        add_subdirectory("${PARADOX_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
    else()
        include(FetchContent)
        FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 51a8a6d7aeae7326956d747737ccf1575e61e209)
        FetchContent_MakeAvailable(JUCE)
    endif()
    function(paradox_plugin target product code synth category)
        juce_add_plugin(${target}
            COMPANY_NAME "PARADOX" BUNDLE_ID "audio.paradox.${target}"
            PLUGIN_MANUFACTURER_CODE Prdx PLUGIN_CODE ${code}
            PRODUCT_NAME "${product}" FORMATS VST3
            IS_SYNTH ${synth} NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
            IS_MIDI_EFFECT FALSE VST3_CATEGORIES ${category}
            COPY_PLUGIN_AFTER_BUILD FALSE)
        target_sources(${target} PRIVATE Source/Plugin/Processor.cpp Source/UI/Editor.cpp)
        target_compile_definitions(${target} PUBLIC
            JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
        target_link_libraries(${target} PRIVATE paradox_engine
            juce::juce_audio_processors juce::juce_gui_extra
            PUBLIC juce::juce_recommended_config_flags)
        if(BUILD_TESTING)
            add_executable(${target}_smoke Tests/PluginSmoke.cpp)
            target_include_directories(${target}_smoke PRIVATE
                $<TARGET_PROPERTY:juce::juce_audio_processors,INTERFACE_INCLUDE_DIRECTORIES>)
            target_compile_definitions(${target}_smoke PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
                JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
            target_link_libraries(${target}_smoke PRIVATE ${target})
            add_test(NAME ${target}_processor COMMAND ${target}_smoke "${product}" "${synth}")
        endif()
    endfunction()
    paradox_plugin(ParadoxInstrument "PARADOX Instrument" PxIn TRUE "Instrument;Synth")
    paradox_plugin(ParadoxEffect "PARADOX Effect" PxFx FALSE "Fx")
endif()
