
#SET(USE_GNUTLS ON CACHE BOOL "Build libdatachannel use gnutls")
#set(USE_GNUTLS ON)
#add_subdirectory(gnutls)

#set(Nettle_LIBRARY C:/gnutls/3.7.8/lib/nettle)
#set(Nettle_INCLUDE_DIR C:/gnutls/3.7.8/lib/nettle)
#set(GNUTLS_LIBRARY C:/gnutls/3.7.8/lib)
#set(GNUTLS_INCLUDE_DIR C:/gnutls/3.7.8/lib/includes/gnutls)
#set(Nettle_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/gnutls/lib/nettle)
#set(Nettle_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gnutls/lib/nettle)
#set(GNUTLS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/gnutls/lib/)
#set(GNUTLS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gnutls/lib/includes)
# -------------------------------- CMakeExtensions --------------------------------
add_subdirectory(CMakeExtensions)

# -------------------------------- GLM -----------------------------------
add_subdirectory(glm/glm)
target_include_directories(glm INTERFACE "glm/glm")
#add_library(glm INTERFACE)
#set_property(TARGET glm PROPERTY FOLDER Dependencies)
#set_target_properties(glm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/glm)
# -------------------------------- arcana --------------------------------
# Dependencies: GSL
add_subdirectory(arcana.cpp)
if(WIN32)
    set_property(TARGET arcana PROPERTY FOLDER Dependencies)
endif()

# -------------------------------- GSL -----------------------------------
# Dependencies: none
add_subdirectory(arcana.cpp/Source/Submodules/GSL)

# -------------------------------- base-n --------------------------------
# Dependencies: none
add_library(base-n INTERFACE)
target_include_directories(base-n INTERFACE "base-n/include")

# -------------------------------- bgfx.cmake --------------------------------
# Dependencies: none
set(BGFX_BUILD_TOOLS OFF CACHE BOOL "")
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "")
set(BGFX_INSTALL OFF CACHE BOOL "")
set(BGFX_CUSTOM_TARGETS OFF CACHE BOOL "")
set(BGFX_USE_DEBUG_SUFFIX OFF CACHE BOOL "")
set(BGFX_OPENGL_USE_EGL ON CACHE BOOL "")
add_subdirectory(bgfx.cmake)
target_compile_definitions(bgfx PRIVATE $<$<CONFIG:Debug>:BGFX_CONFIG_DEBUG=1>)
target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_MULTITHREADED=1)
target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_MAX_VERTEX_STREAMS=18)
target_compile_definitions(bgfx PRIVATE BGFX_GL_CONFIG_BLIT_EMULATION=1)
target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_DEBUG_ANNOTATION=0)
if(GRAPHICS_API STREQUAL "D3D11")
    target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_RENDERER_DIRECT3D11=1)
elseif(GRAPHICS_API STREQUAL "D3D12")
    target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_RENDERER_DIRECT3D12=1)
elseif(GRAPHICS_API STREQUAL "Metal")
    target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_RENDERER_METAL=1)
elseif(GRAPHICS_API STREQUAL "OpenGL")
    target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_RENDERER_OPENGLES=30)
    target_compile_definitions(bgfx PRIVATE BGFX_GL_CONFIG_TEXTURE_READ_BACK_EMULATION=1)
elseif(GRAPHICS_API STREQUAL "Vulkan")
    target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_RENDERER_VULKAN=1)
endif()
set_property(TARGET astc-encoder PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET astc-encoder PROPERTY UNITY_BUILD false)
set_property(TARGET edtaa3 PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET etc1 PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET etc2 PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET iqa PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET nvtt PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET nvtt PROPERTY UNITY_BUILD false)
set_property(TARGET pvrtc PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET squish PROPERTY FOLDER Dependencies/bgfx/3rdparty)
set_property(TARGET squish PROPERTY UNITY_BUILD false)
if(TARGET tinyexr)
    set_property(TARGET tinyexr PROPERTY FOLDER Dependencies/bgfx/3rdparty)
endif()
set_property(TARGET bgfx PROPERTY FOLDER Dependencies/bgfx)
set_property(TARGET bimg PROPERTY FOLDER Dependencies/bgfx)
set_property(TARGET bx PROPERTY FOLDER Dependencies/bgfx)
disable_warnings(astc-encoder)
disable_warnings(edtaa3)
disable_warnings(etc1)
disable_warnings(etc2)
disable_warnings(iqa)
disable_warnings(nvtt)
disable_warnings(pvrtc)
disable_warnings(squish)
disable_warnings(bgfx)
disable_warnings(bimg)
disable_warnings(bx)

if(APPLE)
    set_property(TARGET bgfx PROPERTY UNITY_BUILD false)
endif()

# -------------------------------- glslang --------------------------------
# Dependencies: none
set(BUILD_EXTERNAL OFF CACHE BOOL "")
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "")
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
set(ENABLE_HLSL OFF CACHE BOOL "")
set(ENABLE_OPT OFF CACHE BOOL "")
set(ENABLE_CTEST OFF CACHE BOOL "")
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")
add_subdirectory(glslang)
set_property(TARGET GenericCodeGen PROPERTY FOLDER Dependencies/glslang)
set_property(TARGET glslang PROPERTY FOLDER Dependencies/glslang)
set_property(TARGET OGLCompiler PROPERTY FOLDER Dependencies/glslang)
set_property(TARGET OSDependent PROPERTY FOLDER Dependencies/glslang)
set_property(TARGET MachineIndependent PROPERTY FOLDER Dependencies/glslang)
set_property(TARGET SPIRV PROPERTY FOLDER Dependencies/glslang)
disable_warnings(GenericCodeGen)
disable_warnings(glslang)
disable_warnings(OGLCompiler)
disable_warnings(OSDependent)
disable_warnings(MachineIndependent)
disable_warnings(SPIRV)

# -------------------------------- ios-cmake --------------------------------
# Nothing to do here.

# -------------------------------- napi --------------------------------
# Dependencies: none
add_subdirectory(napi)
set_property(TARGET napi PROPERTY FOLDER Dependencies)

# -------------------------------- SPIRV-Cross --------------------------------
# Dependencies: none
set(SPIRV_CROSS_CLI OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "")
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "")
if(NOT GRAPHICS_API STREQUAL "Metal")
    set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "")
endif()
if(NOT GRAPHICS_API STREQUAL "D3D11" AND NOT GRAPHICS_API STREQUAL "D3D12")
    set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "")
endif()
add_subdirectory(SPIRV-Cross)
set_property(TARGET spirv-cross-core PROPERTY FOLDER Dependencies/SPIRV-Cross)
set_property(TARGET spirv-cross-glsl PROPERTY FOLDER Dependencies/SPIRV-Cross)
disable_warnings(spirv-cross-core)
disable_warnings(spirv-cross-glsl)
if(TARGET spirv-cross-msl)
    set_property(TARGET spirv-cross-msl PROPERTY FOLDER Dependencies/SPIRV-Cross)
endif()
if(TARGET spirv-cross-hlsl)
    set_property(TARGET spirv-cross-hlsl PROPERTY FOLDER Dependencies/SPIRV-Cross)
    disable_warnings(spirv-cross-hlsl)
endif()

# -------------------------------- xr --------------------------------
# Dependencies: none
# Currently supported on Windows via OpenXR, Android via ARCore, and iOS via ARKit.
if(BABYLON_NATIVE_PLUGIN_NATIVEXR AND (WIN32 OR ANDROID OR IOS))
    add_subdirectory(xr)
    set_property(TARGET xr PROPERTY FOLDER Dependencies/xr)
    warnings_as_errors(xr)
    if(WIN32 OR WINDOWS_STORE)
        set_property(TARGET openxr_loader PROPERTY FOLDER Dependencies/xr/OpenXR)
        set_property(TARGET generate_openxr_header PROPERTY FOLDER Dependencies/xr/OpenXR/Generated)
        set_property(TARGET xr_global_generated_files PROPERTY FOLDER Dependencies/xr/OpenXR/Generated)
        set_property(TARGET uninstall PROPERTY FOLDER Dependencies/xr/OpenXR/Generated)
        disable_warnings(openxr_loader)
    endif()
endif()

# -------------------------------- AndroidExtensions --------------------------------
# Dependencies: none
if(ANDROID)
    add_subdirectory(AndroidExtensions)
endif()

# -------------------------------- UrlLib --------------------------------
# Dependencies: arcana, AndroidExtensions
if(BABYLON_NATIVE_POLYFILL_XMLHTTPREQUEST)
    add_subdirectory(UrlLib)
    set_property(TARGET UrlLib PROPERTY FOLDER Dependencies)
endif()

# --------------------------- v8inspector ---------------------------
# Dependencies: none
# Currently only supported on Win32 x86/x64 and Android
if(NAPI_JAVASCRIPT_ENGINE STREQUAL "V8" AND ((WIN32 AND NOT (WINDOWS_STORE OR CMAKE_GENERATOR_PLATFORM_UPPER MATCHES "ARM.*")) OR ANDROID))
    add_subdirectory(v8inspector)
    set_property(TARGET v8inspector PROPERTY FOLDER Dependencies)
    set_property(TARGET v8inspector PROPERTY UNITY_BUILD false)
    set_property(TARGET llhttp PROPERTY UNITY_BUILD false)
endif()

# --------------------------- libdatachannel ---------------------------
SET(NO_MEDIA ON CACHE BOOL "Build libdatachannel media")
#SET(NO_WEBSOCKET ON CACHE BOOL "Build libdatachannel websocket")
SET(NO_EXAMPLES ON CACHE BOOL "Build libdatachannel examples")
SET(NO_TESTS ON CACHE BOOL "Build libdatachannel tests")
#set(ENV{OPENSSL_ROOT_DIR} C:/openssl/3.0.7)
#set(OPENSSL_ROOT_DIR C:/openssl/3.0.7)
add_subdirectory(libdatachannel)
#set_property(TARGET LibDataChannel::LibDataChannel PROPERTY FOLDER Dependencies/libdatachannel)

