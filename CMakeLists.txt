cmake_minimum_required(VERSION 3.22)
project(BBTubeCompressor VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BB_BUILD_VST3 "Build VST3" ON)
option(BB_BUILD_AU "Build Audio Unit on macOS" OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

set(BB_FORMATS "")
if(BB_BUILD_VST3)
    list(APPEND BB_FORMATS VST3)
endif()

if(APPLE AND BB_BUILD_AU)
    list(APPEND BB_FORMATS AU)
endif()

if(NOT BB_FORMATS)
    message(FATAL_ERROR "No plugin format selected.")
endif()

juce_add_plugin(BBTubeCompressor
    COMPANY_NAME "BB Audio Tools"
    COMPANY_WEBSITE "https://example.com"
    COMPANY_EMAIL "support@example.com"
    PLUGIN_MANUFACTURER_CODE BbAt
    PLUGIN_CODE BbTc
    FORMATS ${BB_FORMATS}
    PRODUCT_NAME "BB Tube Compressor"
    PLUGIN_NAME "BB Tube Compressor"
    DESCRIPTION "BB Tube Compressor"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(BBTubeCompressor PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(BBTubeCompressor PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(BBTubeCompressor PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)

juce_generate_juce_header(BBTubeCompressor)
