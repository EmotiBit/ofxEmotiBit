cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11.5" CACHE STRING "Minimum macOS version")

project(SlidePlayer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Paths ──────────────────────────────────────────────────────────────────────
set(OF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")
set(OF_LIBS         "${OF_ROOT}/libs")
set(OF_COMPILED     "${OF_LIBS}/openFrameworksCompiled/lib/osx")
set(OF_THIRD_PARTY  "${OF_COMPILED}/build/output")
set(OF_ADDONS         "${OF_ROOT}/addons")

# ── Shared app sources (excludes main.cpp so tests can reuse without conflict) ─
file(GLOB MM_SOURCES "src/*.mm")
set(OF_APP_SOURCES
    src/ofApp.cpp
    ${MM_SOURCES}
    ${OF_ADDONS}/ofxJSON/src/ofxJSONElement.cpp
    ${OF_ADDONS}/ofxJSON/libs/jsoncpp/src/jsoncpp.cpp
)

# ── Shared compile options ─────────────────────────────────────────────────────
set(OF_COMPILE_OPTIONS
    -stdlib=libc++
    -Wall
    -Werror=return-type
    -fexceptions
    -fpascal-strings
    -fobjc-arc
    -x objective-c++       # OF headers require ObjC++ on macOS
)

# ── Shared compile definitions ─────────────────────────────────────────────────
set(OF_COMPILE_DEFINITIONS
    __MACOSX_CORE__
    USE_FMOD=0
    GL_SILENCE_DEPRECATION=1
    GLES_SILENCE_DEPRECATION=1
    COREVIDEO_SILENCE_GL_DEPRECATION=1
    GLM_FORCE_CTOR_INIT
    GLM_ENABLE_EXPERIMENTAL
)

# ── Shared include directories ─────────────────────────────────────────────────
set(OF_INCLUDE_DIRS
    src
    ${OF_LIBS}/openFrameworks
    ${OF_LIBS}/openFrameworks/3d
    ${OF_LIBS}/openFrameworks/app
    ${OF_LIBS}/openFrameworks/communication
    ${OF_LIBS}/openFrameworks/events
    ${OF_LIBS}/openFrameworks/gl
    ${OF_LIBS}/openFrameworks/graphics
    ${OF_LIBS}/openFrameworks/math
    ${OF_LIBS}/openFrameworks/sound
    ${OF_LIBS}/openFrameworks/types
    ${OF_LIBS}/openFrameworks/utils
    ${OF_LIBS}/openFrameworks/video
    ${OF_LIBS}/freetype/include
    ${OF_LIBS}/freetype/include/freetype2
    ${OF_LIBS}/glew/include
    ${OF_LIBS}/FreeImage/include
    ${OF_LIBS}/tess2/include
    ${OF_LIBS}/cairo/include
    ${OF_LIBS}/rtAudio/include
    ${OF_LIBS}/glfw/include
    ${OF_LIBS}/utf8/include
    ${OF_LIBS}/json/include
    ${OF_LIBS}/glm/include
    ${OF_LIBS}/curl/include
    ${OF_LIBS}/openssl/include
    ${OF_LIBS}/uriparser/include
    ${OF_LIBS}/pugixml/include
    ${OF_LIBS}/brotli/include
    ${OF_ADDONS}/ofxJSON/libs/jsoncpp/include
    ${OF_ADDONS}/ofxJSON/src
)

# ── Shared static libraries ────────────────────────────────────────────────────
set(OF_STATIC_LIBS
    ${OF_COMPILED}/openFrameworks.a
    ${OF_THIRD_PARTY}/FreeImage.a
    ${OF_THIRD_PARTY}/libGLEW.a
    ${OF_THIRD_PARTY}/libfreetype.a
    ${OF_THIRD_PARTY}/libpugixml.a
    ${OF_THIRD_PARTY}/libtess2.a
    ${OF_THIRD_PARTY}/libcairo.a
    ${OF_THIRD_PARTY}/libpixman-1.a
    ${OF_THIRD_PARTY}/librtaudio.a
    ${OF_THIRD_PARTY}/libglfw3.a
    ${OF_THIRD_PARTY}/libpng.a
    ${OF_THIRD_PARTY}/brotli.a
    ${OF_THIRD_PARTY}/curl.a
    ${OF_THIRD_PARTY}/openssl.a
    ${OF_THIRD_PARTY}/uriparser.a
    ${OF_THIRD_PARTY}/libfmt.a
    ${OF_THIRD_PARTY}/zlib.a
    objc
)

# ── macOS Frameworks ───────────────────────────────────────────────────────────
find_library(FW_ACCELERATE           Accelerate)
find_library(FW_APP_KIT              AppKit)
find_library(FW_APPLICATION_SERVICES ApplicationServices)
find_library(FW_AV_FOUNDATION        AVFoundation)
find_library(FW_AUDIO_TOOLBOX        AudioToolbox)
find_library(FW_COCOA                Cocoa)
find_library(FW_CORE_AUDIO           CoreAudio)
find_library(FW_CORE_FOUNDATION      CoreFoundation)
find_library(FW_CORE_MEDIA           CoreMedia)
find_library(FW_CORE_SERVICES        CoreServices)
find_library(FW_CORE_VIDEO           CoreVideo)
find_library(FW_FOUNDATION           Foundation)
find_library(FW_IO_KIT               IOKit)
find_library(FW_METAL                Metal)
find_library(FW_OPEN_GL              OpenGL)
find_library(FW_QUARTZ_CORE          QuartzCore)
find_library(FW_SECURITY             Security)
find_library(FW_SYSTEM_CONFIGURATION SystemConfiguration)

set(OF_FRAMEWORKS
    ${FW_ACCELERATE}
    ${FW_APP_KIT}
    ${FW_APPLICATION_SERVICES}
    ${FW_AV_FOUNDATION}
    ${FW_AUDIO_TOOLBOX}
    ${FW_COCOA}
    ${FW_CORE_AUDIO}
    ${FW_CORE_FOUNDATION}
    ${FW_CORE_MEDIA}
    ${FW_CORE_SERVICES}
    ${FW_CORE_VIDEO}
    ${FW_FOUNDATION}
    ${FW_IO_KIT}
    ${FW_METAL}
    ${FW_OPEN_GL}
    ${FW_QUARTZ_CORE}
    ${FW_SECURITY}
    ${FW_SYSTEM_CONFIGURATION}
)

# ── Main application ───────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp ${OF_APP_SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE ${OF_COMPILE_OPTIONS})
target_compile_definitions(${PROJECT_NAME} PRIVATE ${OF_COMPILE_DEFINITIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${OF_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${OF_STATIC_LIBS} ${OF_FRAMEWORKS})
target_link_options(${PROJECT_NAME} PRIVATE
    -stdlib=libc++
    -mmacosx-version-min=11.5
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

# ── Always copy JSON settings to app bundle Resources ─────────────────────────
set(JSON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/bin/data/emotibitSlidePlayerSettings.json")
set(BUNDLE_RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PROJECT_NAME}.app/Contents/Resources")

set(EXAMPLE_SLIDES_SRC "${CMAKE_CURRENT_SOURCE_DIR}/bin/data/example_slides")

add_custom_target(copy_settings ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_RESOURCES}"
    COMMAND ${CMAKE_COMMAND} -E copy "${JSON_SRC}" "${BUNDLE_RESOURCES}/emotibitSlidePlayerSettings.json"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${EXAMPLE_SLIDES_SRC}" "${BUNDLE_RESOURCES}/example_slides"
    COMMENT "Copying settings and example slides to app bundle Resources"
)
add_dependencies(copy_settings ${PROJECT_NAME})

# ── Tests ──────────────────────────────────────────────────────────────────────
enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(slideShow_tests
    tests/test_parseSettings.cpp
    tests/test_updateCurrentState.cpp
    ${OF_APP_SOURCES}
)

target_compile_options(slideShow_tests PRIVATE ${OF_COMPILE_OPTIONS})
target_compile_definitions(slideShow_tests PRIVATE ${OF_COMPILE_DEFINITIONS})
target_include_directories(slideShow_tests PRIVATE ${OF_INCLUDE_DIRS})
target_link_libraries(slideShow_tests PRIVATE
    Catch2::Catch2WithMain
    ${OF_STATIC_LIBS}
    ${OF_FRAMEWORKS}
)
target_link_options(slideShow_tests PRIVATE
    -stdlib=libc++
    -mmacosx-version-min=11.5
)

include(Catch)
catch_discover_tests(slideShow_tests)
