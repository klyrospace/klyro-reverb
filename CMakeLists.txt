cmake_minimum_required(VERSION 3.22)
project(AuroraReverb VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
set(JUCE_VERSION "8.0.5")
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        ${JUCE_VERSION}
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(AuroraReverb
    VENDOR_NAME            "Inception Labs"
    PLUGIN_MANUFACTURER_CODE "Incp"
    PLUGIN_CODE            "ArvR"
    FORMATS                VST3 AU CLAP
    PRODUCT_NAME           "Aurora Reverb"
    COMPANY_NAME           "Inception Labs"
    IS_SYNTH              FALSE
    NEEDS_MIDI_INPUT      FALSE
    NEEDS_MIDI_OUTPUT     FALSE
    IS_MIDI_EFFECT        FALSE
    EDITOR_NEEDS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(AuroraReverb PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/ReverbProcessor.cpp
)

juce_generate_juce_header(AuroraReverb)
