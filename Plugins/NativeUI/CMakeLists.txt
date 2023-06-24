set(SOURCES
    "Include/Babylon/Plugins/NativeUI.h"
    "Source/NativeUI.cpp")

add_library(NativeUI ${SOURCES})
warnings_as_errors(NativeUI)

if (APPLE)
    set_target_properties(NativeUI PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    )
endif()

target_include_directories(NativeUI PUBLIC "Include")

target_link_to_dependencies(NativeUI
    PUBLIC JsRuntime
    PRIVATE arcana
    PRIVATE napi_extensions
    PRIVATE AndroidExtensions)

set_property(TARGET NativeUI PROPERTY FOLDER Plugins)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
