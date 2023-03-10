cmake_dependent_option(BABYLON_NATIVE_USE_SWAPCHAINPANEL "Use swap chain panel." OFF WINDOWS_STORE OFF)

if(WINDOWS_STORE)
    if(BABYLON_NATIVE_USE_SWAPCHAINPANEL)
        set(PLATFORM_NAME ${BABYLON_NATIVE_PLATFORM}_SwapChainPanel)
        set(PLATFORM_PATH ${BABYLON_NATIVE_PLATFORM}/SwapChainPanel)
    else()
        set(PLATFORM_NAME ${BABYLON_NATIVE_PLATFORM}_CoreWindow)
        set(PLATFORM_PATH ${BABYLON_NATIVE_PLATFORM}/CoreWindow)
    endif()
else()
    set(PLATFORM_NAME ${BABYLON_NATIVE_PLATFORM})
    set(PLATFORM_PATH ${BABYLON_NATIVE_PLATFORM})
endif()

set(SOURCES
    "Include/Platform/${PLATFORM_PATH}/Babylon/Graphics/Platform.h"
    "Include/RendererType/${GRAPHICS_API}/Babylon/Graphics/RendererType.h"
    "Include/Shared/Babylon/Graphics/Device.h"
    "InternalInclude/Babylon/Graphics/BgfxCallback.h"
    "InternalInclude/Babylon/Graphics/continuation_scheduler.h"
    "InternalInclude/Babylon/Graphics/FrameBuffer.h"
    "InternalInclude/Babylon/Graphics/DeviceContext.h"
    "InternalInclude/Babylon/Graphics/SafeTimespanGuarantor.h"
    "InternalInclude/Babylon/Graphics/Texture.h"
    "Source/BgfxCallback.cpp"
    "Source/FrameBuffer.cpp"
    "Source/Device.cpp"
    "Source/DeviceContext.cpp"
    "Source/DeviceImpl.cpp"
    "Source/DeviceImpl.h"
    "Source/DeviceImpl_${PLATFORM_NAME}.${BABYLON_NATIVE_PLATFORM_IMPL_EXT}"
    "Source/DeviceImpl_${GRAPHICS_API}.cpp"
    "Source/SafeTimespanGuarantor.cpp"
    "Source/Texture.cpp")

add_library(Graphics ${SOURCES})
warnings_as_errors(Graphics)

target_include_directories(Graphics
    PRIVATE "Include/Shared"
    PRIVATE "Include/Platform/${PLATFORM_PATH}"
    PRIVATE "Include/RendererType/${GRAPHICS_API}"
    PRIVATE "InternalInclude/Babylon/Graphics")

if(WIN32 AND NOT WINDOWS_STORE)
    target_link_to_dependencies(Graphics
        PRIVATE "shlwapi.lib")
elseif(ANDROID)
    target_link_to_dependencies(Graphics
        PRIVATE android
        PRIVATE AndroidExtensions)
endif()

target_link_to_dependencies(Graphics
    PRIVATE napi_extensions
    PRIVATE JsRuntime
    PRIVATE JsRuntimeInternal
    PRIVATE bgfx
    PRIVATE bimg
    PRIVATE bx)

target_compile_definitions(Graphics
    PRIVATE NOMINMAX)

# TODO: remove this once the #define in GraphicsImpl is removed
target_compile_definitions(Graphics
    PRIVATE $<UPPER_CASE:${GRAPHICS_API}>)

set_property(TARGET Graphics PROPERTY FOLDER Core)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

if(APPLE)
    # compile as objc++
    target_compile_options(Graphics PUBLIC "SHELL:-x objective-c++")
endif()

add_library(GraphicsDevice INTERFACE)
target_include_directories(GraphicsDevice
    INTERFACE "Include/Shared"
    INTERFACE "Include/Platform/${PLATFORM_PATH}"
    INTERFACE "Include/RendererType/${GRAPHICS_API}")
target_link_to_dependencies(GraphicsDevice
    INTERFACE Graphics
    INTERFACE JsRuntime)

add_library(GraphicsDeviceContext INTERFACE)
target_include_directories(GraphicsDeviceContext
    INTERFACE "InternalInclude"
    INTERFACE "InternalInclude/${BABYLON_NATIVE_PLATFORM}")
target_link_to_dependencies(GraphicsDeviceContext
    INTERFACE Graphics
    INTERFACE JsRuntimeInternal
    INTERFACE arcana
    INTERFACE bgfx
    INTERFACE bimg
    INTERFACE bx)

target_compile_definitions(GraphicsDeviceContext
    INTERFACE NOMINMAX)
