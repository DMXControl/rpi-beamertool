cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(rpi-beamertool
  VERSION 0.4.0)

# Set a default build type if none was specified
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()

# Dependencies
find_package(Threads REQUIRED)

find_package(Curses REQUIRED)

# Source files
set(source_files
  src/rpi_beamertool.cpp
  src/artnet_receiver.cpp
  src/config_loader.cpp
  src/console_screen.cpp
  src/canvas/canvas_manager.cpp
  src/canvas/canvas.cpp
  src/canvas/renderer.cpp
  src/canvas/fps_counter.cpp
  src/canvas/canvas_content/content_manager.cpp
  src/canvas/canvas_content/content_interface.cpp
  src/canvas/canvas_content/content_example_plugin/content_example_plugin.cpp
  src/canvas/canvas_content/content_local_file_texture/content_local_file_texture.cpp
  src/canvas/canvas_content/content_local_file_texture/texture.cpp
  src/canvas/canvas_content/content_local_file_texture/stb_image.c
  src/canvas/canvas_content/content_local_file_texture/GIFLoader.cpp
  src/canvas/canvas_content/content_local_file_texture/video_player.cpp)

# Header files
set(header_files
  src/artnet_receiver.h
  src/config_loader.h
  src/console_screen.h
  src/canvas/canvas_classes.h
  src/canvas/canvas_manager.h
  src/canvas/canvas.h
  src/canvas/renderer.h
  src/canvas/fps_counter.h
  src/canvas/canvas_content/content_manager.h
  src/canvas/canvas_content/content_interface.h
  src/canvas/canvas_content/content_example_plugin/content_example_plugin.h
  src/canvas/canvas_content/content_local_file_texture/content_local_file_texture.h
  src/canvas/canvas_content/content_local_file_texture/texture.h
  src/canvas/canvas_content/content_local_file_texture/stb_image.h
  src/canvas/canvas_content/content_local_file_texture/GIFLoader.h
  src/canvas/canvas_content/content_local_file_texture/video_player.h)

# Project
add_executable(${PROJECT_NAME}
  ${source_files} ${header_files})

# C++ standart
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

# Including and linking
target_include_directories(${PROJECT_NAME} PRIVATE
  /opt/vc/include/
  /opt/vc/src/hello_pi/libs/ilclient
  ${CURSES_INCLUDE_DIR})
target_link_directories(${PROJECT_NAME} PRIVATE
  /opt/vc/lib
  /opt/vc/src/hello_pi/libs/ilclient)
target_link_libraries(${PROJECT_NAME} PRIVATE
  Threads::Threads
  brcmGLESv2
  brcmEGL
  openmaxil
  bcm_host
  vcos
  ilclient
  ${CURSES_LIBRARIES})

# Raspberry Pi specific settings from Makefile
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DVERSION_STRING=\"${PROJECT_VERSION}\"" -DOMX -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM)

# Install
# TODO

# Package
# TODO setup CPack

# Enable strict warnings
option(RPI_BEAMERTOOL_STRICT_WARNINGS "Enable strict compiler warnings." OFF)
if (RPI_BEAMERTOOL_STRICT_WARNINGS)
  target_compile_options(${PROJECT_NAME} PRIVATE -Werror -Wall -Wextra -pedantic -pedantic-errors)
endif ()
