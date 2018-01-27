cmake_minimum_required (VERSION 3.1)

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

set(PACKAGE_VERSION "0.2.0")
message (STATUS "Building Midiguitar version ${PACKAGE_VERSION} for ${CMAKE_SYSTEM_NAME}")

add_definitions (-D'PACKAGE_VERSION="${PACKAGE_VERSION}"')
add_definitions(-DCMAKE_BUILD=1)
project(midiguitar)

add_executable(midiguitar src/main.cpp src/guitar.cpp src/RtMidi.cpp)
target_link_libraries (midiguitar -ldl)

option (EnableRtMidi "Enable RtMidi build. RtMidi is the default build" ON)
option (EnableJackStandalone "Enable jack standalone support" OFF)
option (EnableAlsaLegacy "Enable legacy alsa support" OFF)
option (BuildForDebug "Include gdb debugging support" OFF)

find_package (PkgConfig REQUIRED)

# dependencies
find_package (ALSA REQUIRED)
pkg_check_modules (JACK REQUIRED jack>=0.115.6)

# fltk
find_package (FLTK REQUIRED)
if (FLTK_FOUND)
    include_directories(${FLTK_INCLUDE_DIR})
    target_link_libraries(midiguitar ${FLTK_LIBRARIES})
endif (FLTK_FOUND)


find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  set_property(TARGET midiguitar PROPERTY COMPILE_OPTIONS "-pthread")
  set_property(TARGET midiguitar PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(midiguitar "${CMAKE_THREAD_LIBS_INIT}")
endif()

if (BuildForDebug)
    set (CMAKE_BUILD_TYPE "Debug")
    set (CMAKE_CXX_FLAGS_DEBUG ${BuildOptionsDebug})
    message (STATUS "Building for ${CMAKE_BUILD_TYPE}, flags: ${CMAKE_CXX_FLAGS_DEBUG}")
else (BuildForDebug)
    set (CMAKE_BUILD_TYPE "Release")
endif (BuildForDebug)

if (EnableRtMidi)
    add_definitions(-DRTMIDI_SUPPORT=1)
    if (JACK_FOUND)
        include_directories(${JACK_INCLUDE_DIR})
        target_link_libraries(midiguitar ${JACK_LIBRARIES})
        add_definitions(-D__UNIX_JACK__=1)
    endif(JACK_FOUND)
    if (ALSA_FOUND)
        include_directories(${ALSA_INCLUDE_DIR})
        target_link_libraries(midiguitar ${ALSA_LIBRARIES})
        add_definitions(-D__LINUX_ALSA__=1)
    endif(ALSA_FOUND)
endif (EnableRtMidi)

if (EnableJackStandalone)
    if (JACK_FOUND)
        add_definitions(-DJACK_SUPPORT=1)
        if(NOT EnableRtMidi)   # These are already included if RtMidi is enabled
            include_directories(${JACK_INCLUDE_DIR})
            target_link_libraries(midiguitar ${JACK_LIBRARIES})
        endif (NOT EnableRtMidi)
    endif (JACK_FOUND)
endif (EnableJackStandalone)

if (EnableAlsaLegacy)
    if (ALSA_FOUND)
        add_definitions(-DALSA_SUPPORT=1)
        if(NOT EnableRtMidi)   # These are already included if RtMidi is enabled
            include_directories(${ALSA_INCLUDE_DIR})
            target_link_libraries(midiguitar ${ALSA_LIBRARIES})
        endif (NOT EnableRtMidi)
    endif (ALSA_FOUND)
endif (EnableAlsaLegacy)

# clean sweep
add_custom_target (distclean
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/*.log
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/Makefile
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/install_manifest.txt
    COMMAND rm -f ${CMAKE_SOURCE_DIR}/cmake_install.cmake
    COMMAND find ${CMAKE_SOURCE_DIR} -type f -name CMakeCache.txt | xargs -r rm -f
    COMMAND find ${CMAKE_SOURCE_DIR} -type d -name CMakeFiles | xargs -r rm -rf
    COMMAND find ${CMAKE_SOURCE_DIR} -type f -name "*.marks" | xargs -r rm -f
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

install(TARGETS midiguitar RUNTIME DESTINATION bin)
install (FILES midiguitar.desktop
    DESTINATION share/applications)
install (FILES midi_guitar_48.png
    DESTINATION share/pixmaps)
