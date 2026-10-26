cmake_minimum_required(VERSION 3.22)

project(DembowClasico VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    # Binario universal (Apple Silicon + Intel), macOS 11 o más nuevo
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
    if (NOT CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    endif()
endif()

# ---------------------------------------------------------------------------------------------
# JUCE: se descarga solo. Si ya lo tienes, pasa -DDMB_JUCE_PATH=/ruta/a/JUCE
set(DMB_JUCE_PATH "" CACHE PATH "Ruta a una copia local de JUCE (opcional)")

if (DMB_JUCE_PATH)
    add_subdirectory(${DMB_JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.15
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# ---------------------------------------------------------------------------------------------
# Nombre, fabricante e identificador salen de product.env (lo mismo usa el instalador).
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/product.env" DMB_ENV_LINES REGEX "^[A-Z_]+=")
foreach(line IN LISTS DMB_ENV_LINES)
    if (line MATCHES "^([A-Z_]+)=\"?([^\"]*)\"?$")
        set(DMB_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}")
    endif()
endforeach()
string(TIMESTAMP DMB_YEAR "%Y")

set(DMB_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND DMB_FORMATS AU)
endif()

# En Mac copiamos el plugin a ~/Library/Audio/Plug-Ins al compilar (no en servidores de CI).
set(DMB_COPY_AFTER_BUILD FALSE)
if (APPLE AND NOT DEFINED ENV{CI})
    set(DMB_COPY_AFTER_BUILD TRUE)
endif()

juce_add_plugin(DembowClasico
    COMPANY_NAME            "${DMB_COMPANY_NAME}"
    COMPANY_WEBSITE         "${DMB_WEBSITE}"
    COMPANY_EMAIL           "${DMB_CONTACT_EMAIL}"
    COMPANY_COPYRIGHT       "Copyright ${DMB_YEAR} ${DMB_COMPANY_NAME}. Todos los derechos reservados."
    BUNDLE_ID               "${DMB_BUNDLE_ID}"
    PLUGIN_MANUFACTURER_CODE Ecoa
    PLUGIN_CODE             Dmbw
    FORMATS                 ${DMB_FORMATS}
    PRODUCT_NAME            "${DMB_PRODUCT_NAME}"
    DESCRIPTION             "Synth de reggaeton old school"
    ICON_BIG                "${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon/icon.png"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${DMB_COPY_AFTER_BUILD}
    VST3_CATEGORIES         Instrument Synth
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice)

set(DMB_SOURCES
    Source/I18n.cpp
    Source/Params.cpp
    Source/synth/Patch.cpp
    Source/synth/Voice.cpp
    Source/synth/Synth.cpp
    Source/synth/Effects.cpp
    Source/presets/Factory.cpp
    Source/presets/Leads.cpp
    Source/presets/Keys.cpp
    Source/presets/Orchestra.cpp
    Source/presets/Others.cpp
    Source/ui/Widgets.cpp
    Source/ui/PresetBrowser.cpp
    Source/ui/Keyboard.cpp
    Source/Updater.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_sources(DembowClasico PRIVATE ${DMB_SOURCES})

# Imágenes y tipografía de la interfaz (se incrustan dentro del plugin)
juce_add_binary_data(DmbAssets
    HEADER_NAME DmbAssets.h
    NAMESPACE   DmbAssets
    SOURCES
        Resources/ui/plate.jpg
        Resources/ui/drawer_bg.jpg
        Resources/fonts/Barlow-Medium.ttf
        Resources/fonts/Barlow-SemiBold.ttf)
set_target_properties(DmbAssets PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(DembowClasico PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    "DMB_UPDATE_URL=\"${DMB_UPDATE_URL}\""
    "DMB_DISPLAY_NAME=\"${DMB_DISPLAY_NAME}\""
    DMB_YEAR=${DMB_YEAR})

# Los textos tienen acentos: en Windows hay que decirle al compilador que el código está en UTF-8.
if (MSVC)
    target_compile_options(DembowClasico PRIVATE /utf-8)
endif()

target_link_libraries(DembowClasico
    PRIVATE
        DmbAssets
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# ---------------------------------------------------------------------------------------------
# Herramienta de pruebas (opcional): verifica el sonido, los presets y genera capturas.
option(DMB_BUILD_TESTS "Compilar la herramienta de pruebas" OFF)

if (DMB_BUILD_TESTS)
    juce_add_console_app(DembowTests PRODUCT_NAME "DembowTests")
    target_sources(DembowTests PRIVATE tests/DembowTests.cpp ${DMB_SOURCES})
    target_include_directories(DembowTests PRIVATE Source)
    target_compile_definitions(DembowTests PRIVATE
        "JucePlugin_Name=\"${DMB_PRODUCT_NAME}\""
        "JucePlugin_Manufacturer=\"${DMB_COMPANY_NAME}\""
        "JucePlugin_VersionString=\"${PROJECT_VERSION}\""
        "DMB_UPDATE_URL=\"${DMB_UPDATE_URL}\""
        "DMB_DISPLAY_NAME=\"${DMB_DISPLAY_NAME}\""
        DMB_YEAR=${DMB_YEAR}
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_USE_FLAC=0
        JUCE_USE_OGGVORBIS=0
        JUCE_PLUGINHOST_VST3=1)   # para cargar el .vst3 ya compilado como lo haría un DAW
    if (MSVC)
        target_compile_options(DembowTests PRIVATE /utf-8)
    endif()
    target_link_libraries(DembowTests PRIVATE
        DmbAssets
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
endif()
