cmake_minimum_required(VERSION 3.14)
project(video-app C CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(lib/FFmpeg)
find_package(raylib REQUIRED)

add_definitions(-DGL_SILENCE_DEPRECATION)

list(APPEND SOURCES
    src/main.cpp
)

add_executable(video-app MACOSX_BUNDLE WIN32 ${SOURCES}
        src/test.h
        src/test.cpp
        src/define.h
        src/extension.h
        src/extension.cpp
        src/Bar.cpp
        src/Bar.h
        src/plug.cpp
        src/plug.h
        src/helpers.h
        src/PCM.cpp
        src/PCM.h
        src/draw.cpp
        src/draw.h
)

if(APPLE)
    list(APPEND EXTRA_LIBS
            "-framework OpenGL"
    )

    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/MacOSXBundleInfo.plist.in
            ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
    )

    set_target_properties(video-app PROPERTIES
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/assets/MacOSXBundleInfo.plist
    )

elseif(WIN32)
    list(APPEND EXTRA_LIBS
            "-lglu32 -lopengl32"
    )
    set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -static -static-libgcc -static-libstdc++ -mwindows")
else()
    list(APPEND EXTRA_LIBS
            "-lGL -lGLU -lX11"
    )
endif()

target_link_libraries(video-app
    FFmpeg
    ${EXTRA_LIBS}
    fftw3
    raylib
)
