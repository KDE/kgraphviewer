project(kgraphviewer)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH})

set(KGRAPHVIEWER_VERSION_MAJOR "2")
set(KGRAPHVIEWER_VERSION_MINOR "1")
set(KGRAPHVIEWER_VERSION_PATCH "90")
set(KGRAPHVIEWER_SOVERSION "${KGRAPHVIEWER_VERSION_MAJOR}")
set(KGRAPHVIEWER_LIB_VERSION "${KGRAPHVIEWER_VERSION_MAJOR}.${KGRAPHVIEWER_VERSION_MINOR}")
configure_file (config-kgraphviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgraphviewer.h )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

find_package(Boost 1.36 REQUIRED)
find_package(GraphViz REQUIRED)

if(NOT DOT)
   MESSAGE(STATUS "dot executable from GraphViz not found. You will need it at runtime")
endif(NOT DOT)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

remove_definitions(-DQT_NO_STL) 

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    ${graphviz_INCLUDE_DIRECTORIES}
    ${INCLUDE_INSTALL_DIR}
)

add_subdirectory(src)
add_subdirectory(doc)

