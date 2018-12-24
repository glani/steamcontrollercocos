cmake_minimum_required(VERSION 3.1)

project(SteamControllerCocos)

set(STEAMCONTROLLERCOCOS_LIB steamcontrollercocos)

set(STEAMCONTROLLERCOCOS_SRC SteamController.cpp SteamControllerImpl-win32-linux.cpp EventListenerSteamController.cpp EventSteamController.cpp SteamControllerCommons.h)

set(STEAMCONTROLLERCOCOS_HEADER SteamController.h SteamControllerImpl-win32-linux.h EventListenerSteamController.h EventSteamController.h)

list(APPEND STEAMCONTROLLERCOCOS_SRC ${STEAMCONTROLLERCOCOS_HEADER})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/sclib/include
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sclib)
add_library(${STEAMCONTROLLERCOCOS_LIB} STATIC ${STEAMCONTROLLERCOCOS_SRC})
target_link_libraries(${STEAMCONTROLLERCOCOS_LIB} sclib)

set_target_properties(${STEAMCONTROLLERCOCOS_LIB}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        )