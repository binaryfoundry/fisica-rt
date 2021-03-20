cmake_minimum_required(VERSION 3.8)

set(PROJECT_NAME fisica-rt)
set(PROJECT_files_NAME fisica-rt-files)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(EMSCRIPTEN "Web Compilation" OFF)

set(SOURCES
    src/Application.cpp
    src/Context.cpp
    src/Camera.cpp
    src/Geometry.cpp
    src/Noise.cpp
    src/Timing.cpp
    src/Input.cpp
    src/File.cpp
    src/Parsing.cpp)

set(HEADERS
    src/Application.hpp
    src/Context.hpp
    src/Camera.hpp
    src/Geometry.hpp
    src/Noise.hpp
    src/Timing.hpp
    src/Input.hpp
    src/File.hpp
    src/Parsing.hpp)

set(SOURCES_INTERFACES
    src/interfaces/IApplication.cpp)

set(HEADERS_INTERFACES
    src/interfaces/IApplication.hpp)

set(SOURCES_PIPELINES
    src/pipelines/Raytracing.cpp)

set(HEADERS_PIPELINES
    src/pipelines/Raytracing.hpp)

set(SOURCES_MATH
    src/math/Math.cpp
    src/math/Angles.cpp
    src/math/Random.cpp)

set(HEADERS_MATH
    src/math/Math.hpp
    src/math/Angles.hpp
    src/math/Random.hpp)

set(SOURCES_SDL
    src/sdl/SDL.cpp
    src/sdl/Main.cpp
    src/sdl/MainWeb.cpp
    src/sdl/Imgui.cpp)

set(HEADERS_SDL
    src/sdl/SDL.hpp
    src/sdl/Main.hpp
    src/sdl/MainWeb.hpp
    src/sdl/Imgui.hpp)

set(SOURCES_GL
    src/gl/OpenGL.cpp
    src/gl/ImGui.cpp
    src/gl/FrameBuffer.cpp
    src/gl/Texture2D.cpp
    src/gl/UniformBuffer.cpp
    src/gl/Pipeline.cpp
    src/gl/Parser.cpp
    src/gl/Shader.cpp
    src/gl/Descriptor.cpp)

set(HEADERS_GL
    src/gl/OpenGL.hpp
    src/gl/ImGui.hpp
    src/gl/FrameBuffer.hpp
    src/gl/Texture2D.hpp
    src/gl/UniformBuffer.hpp
    src/gl/Pipeline.hpp
    src/gl/Parser.hpp
    src/gl/Shader.hpp
    src/gl/Descriptor.hpp)

set(SOURCES_PROPERTIES
    src/properties/Interpolator.cpp
    src/properties/Easing.cpp
    src/properties/IProperty.cpp
    src/properties/Property.cpp
    src/properties/Manager.cpp)

set(HEADERS_PROPERTIES
    src/properties/Interpolator.hpp
    src/properties/Easing.hpp
    src/properties/IProperty.hpp
    src/properties/Property.hpp
    src/properties/Manager.hpp)

set(SOURCES_IMGUI
    lib/imgui/imgui/imgui.cpp
    lib/imgui/imgui/imgui_draw.cpp
    lib/imgui/imgui/imgui_demo.cpp
    lib/imgui/imgui/imgui_widgets.cpp)

set(HEADERS_IMGUI
    lib/imgui/imgui/imconfig.h
    lib/imgui/imgui/imgui.h
    lib/imgui/imgui/imgui_internal.h
    lib/imgui/imgui/imstb_rectpack.h
    lib/imgui/imgui/imstb_textedit.h
    lib/imgui/imgui/imstb_truetype.h)

SOURCE_GROUP("Source" FILES ${SOURCES})
SOURCE_GROUP("Source" FILES ${HEADERS})

SOURCE_GROUP("Source\\interfaces" FILES ${SOURCES_INTERFACES})
SOURCE_GROUP("Source\\interfaces" FILES ${HEADERS_INTERFACES})

SOURCE_GROUP("Source\\pipelines" FILES ${SOURCES_PIPELINES})
SOURCE_GROUP("Source\\pipelines" FILES ${HEADERS_PIPELINES})

SOURCE_GROUP("Source\\math" FILES ${SOURCES_MATH})
SOURCE_GROUP("Source\\math" FILES ${HEADERS_MATH})

SOURCE_GROUP("Source\\sdl" FILES ${SOURCES_SDL})
SOURCE_GROUP("Source\\sdl" FILES ${HEADERS_SDL})

SOURCE_GROUP("Source\\gl" FILES ${SOURCES_GL})
SOURCE_GROUP("Source\\gl" FILES ${HEADERS_GL})

SOURCE_GROUP("Source\\properties" FILES ${SOURCES_PROPERTIES})
SOURCE_GROUP("Source\\properties" FILES ${HEADERS_PROPERTIES})

SOURCE_GROUP("Source\\imgui" FILES ${SOURCES_IMGUI})
SOURCE_GROUP("Source\\imgui" FILES ${HEADERS_IMGUI})

include_directories(
    ${PROJECT_SOURCE_DIR}/lib/glm
    ${PROJECT_SOURCE_DIR}/lib/stb
    ${PROJECT_SOURCE_DIR}/lib/imgui
    ${PROJECT_SOURCE_DIR}/lib/noise)

if (WIN32)
    message(STATUS "Platform: Windows")
    add_compile_options("/EHsc")

    set(SDL2_PATH ${PROJECT_SOURCE_DIR}/lib/sdl/win/sdl2)
    set(SDL2_LIB_PATH ${SDL2_PATH}/lib/x64)

    set(SDL2_IMAGE_PATH ${PROJECT_SOURCE_DIR}/lib/sdl/win/sdl2-image)
    set(SDL2_IMAGE_LIB_PATH ${SDL2_IMAGE_PATH}/lib/x64)

    set(PVR_PATH ${PROJECT_SOURCE_DIR}/lib/pvr)
    set(PVR_LIB_PATH ${PVR_PATH}/Windows/x86_64)

    include_directories(${SDL2_PATH}/include)
    link_directories(${SDL2_LIB_PATH})

    include_directories(${SDL2_IMAGE_PATH}/include)
    link_directories(${SDL2_IMAGE_LIB_PATH})

    include_directories(${PVR_PATH}/Include)
    link_directories(${PVR_LIB_PATH})

elseif (EMSCRIPTEN)
    message(STATUS "Platform: HTML5")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")

endif ()

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${SOURCES_INTERFACES}
    ${HEADERS_INTERFACES}
    ${SOURCES_PIPELINES}
    ${HEADERS_PIPELINES}
    ${SOURCES_MATH}
    ${HEADERS_MATH}
    ${SOURCES_SDL}
    ${HEADERS_SDL}
    ${SOURCES_GL}
    ${HEADERS_GL}
    ${SOURCES_PROPERTIES}
    ${HEADERS_PROPERTIES}
    ${SOURCES_IMGUI}
    ${HEADERS_IMGUI})

add_custom_target(
    ${PROJECT_files_NAME} ALL
    COMMENT "Copying Files..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/files/
    ${CMAKE_CURRENT_BINARY_DIR}/files)

add_dependencies(
    ${PROJECT_NAME}
    ${PROJECT_files_NAME})

if (WIN32)
   target_include_directories(
        ${PROJECT_NAME}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include/
        ${SDL2_PATH}/include
        ${SDL2_IMAGE_PATH}/include
        ${PVR_PATH}/include)

    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        SDL2
        SDL2main
        SDL2_image
        libEGL
        libGLESv2
        libGLES_CM)

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SDL2_LIB_PATH}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SDL2_IMAGE_LIB_PATH}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PVR_LIB_PATH}"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

elseif (EMSCRIPTEN)

    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS "--bind -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s USE_WEBGL2=1 -s WASM=1 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s --emrun -std=c++11 -O3 --preload-file files --shell-file \"${PROJECT_SOURCE_DIR}/shell.html\" --use-preload-plugins --no-heap-copy")

endif ()
