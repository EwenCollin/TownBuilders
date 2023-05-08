set(SOURCES
    "Include/Babylon/Plugins/NativeRTC.h"
    "Source/NativeRTC.cpp")

add_library(NativeRTC ${SOURCES})
warnings_as_errors(NativeRTC)

if (APPLE)
    set_target_properties(NativeRTC PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    )
endif()

target_include_directories(NativeRTC PUBLIC "Include")

target_link_to_dependencies(NativeRTC
    PUBLIC JsRuntime
    PRIVATE arcana
    PRIVATE napi_extensions
    PRIVATE LibDataChannel::LibDataChannel)

set_property(TARGET NativeRTC PROPERTY FOLDER Plugins)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
