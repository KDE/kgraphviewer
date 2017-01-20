project(kgraphviewer)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMInstallIcons)

set(KGRAPHVIEWER_VERSION_MAJOR "2")
set(KGRAPHVIEWER_VERSION_MINOR "2")
set(KGRAPHVIEWER_VERSION_PATCH "0")
set(KGRAPHVIEWER_SOVERSION "${KGRAPHVIEWER_VERSION_MAJOR}")
set(KGRAPHVIEWER_LIB_VERSION "${KGRAPHVIEWER_VERSION_MAJOR}.${KGRAPHVIEWER_VERSION_MINOR}")
configure_file (config-kgraphviewer.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kgraphviewer.h )

# search packages used by KDE
find_package(Qt5 CONFIG REQUIRED Core DBus Widgets Svg PrintSupport)

find_package(Boost 1.36 REQUIRED)
find_package(GraphViz REQUIRED)

find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    DocTools
    Parts
    KIO
    WidgetsAddons
    IconThemes
)

if(NOT DOT)
   MESSAGE(STATUS "dot executable from GraphViz not found. You will need it at runtime")
endif(NOT DOT)

add_definitions (${QT_DEFINITIONS})

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${graphviz_INCLUDE_DIRECTORIES}
    ${KDE_INSTALL_INCLUDEDIR}
)

add_subdirectory(src)
add_subdirectory(doc)

