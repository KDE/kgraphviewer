project(kgraphviewer)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH})


# search packages used by KDE
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

# are these two really required ?
find_package(ZLIB REQUIRED)
if (APPLE)
   find_package(Carbon REQUIRED)
endif (APPLE)

find_package(Boost 1.36 REQUIRED)
find_package(GraphViz REQUIRED)

if(NOT DOT)
   MESSAGE(STATUS "dot executable from GraphViz not found. You will need it at runtime")
endif(NOT DOT)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT3_SUPPORT)

remove_definitions(-DQT_NO_STL) 

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})


configure_file (config-kgraphviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgraphviewer.h )

add_subdirectory(src)
