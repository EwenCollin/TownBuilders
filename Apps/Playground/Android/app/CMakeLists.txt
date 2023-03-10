cmake_minimum_required(VERSION 3.13.2)
# set targetPlatform, will be passed in from gradle when this sample is completed
# openGL Supportability
# platform         status
#   (0 12)           ES2/ES3 not supported
#   [12, 18)         ES2 only; for ES3, app do dynamic load/detection
#                    this applies to the situations that:
#                        - minimum API is set to less than 18. In this case
#                          there is no ES3 header/lib support inside NDK
#                        - the built APK might be running on newer API phones
#                    with dynamic loading of ES3, the same APK would still be able
#                    to use ES3. Otherwise, app would stuck with ES2 even phone is
#                    is newer than the minimum API level (for example, Android-27 etc).
#
#   [18, 24)         ES2 & ES3
#                    If app is built to only support API-18 or later,
#                    set minimum api level to 18 is good enough, NDK supprts ES3
#                    with the right header and lib files. No need to use ES3 dynamic
#                    detection.
#   [24, infinite)   ES2 & ES3 & Vulkan
project(BabylonNative)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(BABYLON_NATIVE_PLATFORM "Android")
set(CMAKE_CXX_STANDARD 17)

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(openssl REQUIRED CONFIG)


if (${ANDROID_JSENGINE_LIBNAME} STREQUAL "jsc")
    set(NAPI_JAVASCRIPT_ENGINE "JavaScriptCore" CACHE STRING "JavaScript engine for N-API.")
elseif (${ANDROID_JSENGINE_LIBNAME} STREQUAL "v8android")
    set(NAPI_JAVASCRIPT_ENGINE "V8" CACHE STRING "JavaScript engine for N-API.")
else()
    message(FATAL_ERROR "Unrecognized JS engine: ${ANDROID_JSENGINE_LIBNAME}")
endif()

add_library(javascript_engine SHARED IMPORTED GLOBAL)
set_target_properties(javascript_engine PROPERTIES IMPORTED_LOCATION "${ANDROID_JSENGINE_LIBPATH}/${ANDROID_ABI}/lib${ANDROID_JSENGINE_LIBNAME}.so")
target_include_directories(javascript_engine INTERFACE "${ANDROID_JSENGINE_INCPATH}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/src/")

set(BABYLON_NATIVE_PLAYGROUND_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
add_subdirectory(${BABYLON_NATIVE_PLAYGROUND_DIR}/../../ ${BABYLON_NATIVE_PLAYGROUND_DIR}/../../build/Android_${CMAKE_ANDROID_ARCH_ABI}/)

add_library(BabylonNativeJNI SHARED
            src/main/cpp/BabylonNativeJNI.cpp)

target_include_directories(BabylonNativeJNI
        PRIVATE
        ${BABYLON_NATIVE_PLAYGROUND_DIR}/Shared)

#add_library(openssl SHARED IMPORTED GLOBAL)

target_link_libraries(BabylonNativeJNI
        glm
        GLESv3
        android
        EGL
        log
        -lz
        AppRuntime
        NativeEngine
        NativeInput
        NativeXr
        NativeOptimizations
        Console
        Canvas
        Window
        ScriptLoader
        XMLHttpRequest
        NativeCamera
        AndroidExtensions
        ChromeDevTools
        NativeRTC
        openssl::crypto
        openssl::ssl
        )
