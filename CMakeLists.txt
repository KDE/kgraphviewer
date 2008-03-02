project(kgraphviewer)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules )

# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

# are these two really required ?
find_package(ZLIB REQUIRED)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)
find_package(KdepimLibs REQUIRED)
#macro_optional_find_package(GraphViz)

FIND_PROGRAM(DOT_EXECUTABLE NAMES dot  PATHS /usr/bin/  )
if(NOT DOT_EXECUTABLE)
   MESSAGE(STATUS "GraphViz not found. You will need it at runtime")
endif(NOT DOT_EXECUTABLE)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT3_SUPPORT)

remove_definitions(-DQT_NO_STL) 

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${Boost_INCLUDE_DIR})


configure_file (config-kgraphviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgraphviewer.h )

add_subdirectory(src)
