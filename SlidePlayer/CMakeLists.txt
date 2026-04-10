cmake_minimum_required(VERSION 3.20)

set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(SlidePlayer)

# ── Shared OF + addon modules ─────────────────────────────────────────────────
set(_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")
include("${_CMAKE_DIR}/openFrameworks.cmake")
include("${_CMAKE_DIR}/addons/ofxJSON.cmake")

# ── Shared app sources (no main.cpp — reused by both app and tests) ───────────
file(GLOB MM_SOURCES "src/*.mm")
set(OF_APP_SOURCES
    src/ofApp.cpp
    ${MM_SOURCES}
    ${ADDON_OFXJSON_SOURCES}
)

# ── Main application ──────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} MACOSX_BUNDLE src/main.cpp ${OF_APP_SOURCES})

of_configure_app(${PROJECT_NAME} 23)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${ADDON_OFXJSON_INCLUDE_DIRS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "SlidePlayer"
)

# ── Copy settings + slides to app bundle Resources ────────────────────────────
set(BUNDLE_RESOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PROJECT_NAME}.app/Contents/Resources")
add_custom_target(copy_settings ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_RESOURCES}"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/data/emotibitSlidePlayerSettings.json"
        "${BUNDLE_RESOURCES}/emotibitSlidePlayerSettings.json"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/bin/data/example_slides"
        "${BUNDLE_RESOURCES}/example_slides"
    COMMENT "Copying settings and example slides to app bundle Resources"
)
add_dependencies(copy_settings ${PROJECT_NAME})

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

add_executable(slideShow_tests
    tests/main.cpp
    tests/test_parseSettings.cpp
    tests/test_updateCurrentState.cpp
    ${OF_APP_SOURCES}
)

of_configure_app(slideShow_tests 23)

# Keep test binary in the build dir, not bin/ — it can't run as a subprocess
# with no display context (links against AppKit/Cocoa via OF)
set_target_properties(slideShow_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

target_include_directories(slideShow_tests PRIVATE
    ${ADDON_OFXJSON_INCLUDE_DIRS}
)

target_link_libraries(slideShow_tests PRIVATE Catch2::Catch2)

include(Catch)
# PRE_TEST: discover tests when ctest runs, not at build time
catch_discover_tests(slideShow_tests DISCOVERY_MODE PRE_TEST)
