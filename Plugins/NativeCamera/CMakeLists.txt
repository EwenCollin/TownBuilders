if(APPLE)
    set(PLATFORM Apple)
else()
    set(PLATFORM ${BABYLON_NATIVE_PLATFORM})
endif()

set(SOURCES
    "Include/Babylon/Plugins/NativeCamera.h"
    "Source/Capability.h"
    "Source/Capability.cpp"
    "Source/Constraint.h"
    "Source/Constraint.cpp"
    "Source/MediaDevices.h"
    "Source/MediaDevices.cpp"
    "Source/MediaStream.h"
    "Source/MediaStream.cpp"
    "Source/NativeCamera.cpp"
    "Source/NativeVideo.cpp"
    "Source/NativeVideo.h"
    "Source/CameraDevice.h"
    "Source/CameraDeviceSharedPImpl.h"
    "Source/${PLATFORM}/CameraDevice.${BABYLON_NATIVE_PLATFORM_IMPL_EXT}")

add_library(NativeCamera ${SOURCES})
warnings_as_errors(NativeCamera)

target_include_directories(NativeCamera
    INTERFACE "Include"
    PRIVATE "Include/Babylon/Plugins"
    PRIVATE "Source/${PLATFORM}")

if(ANDROID)
    set(EXTENSIONS AndroidExtensions)
elseif(APPLE)
    set(EXTENSIONS "-framework CoreMedia -framework AVFoundation")
    set_target_properties(NativeCamera PROPERTIES
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    )
endif()

target_link_to_dependencies(NativeCamera
    PUBLIC JsRuntime
    PRIVATE GraphicsDeviceContext
    PRIVATE napi_extensions
    PRIVATE ${EXTENSIONS})

set_property(TARGET NativeCamera PROPERTY FOLDER Plugins)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
