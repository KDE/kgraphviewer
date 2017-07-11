cmake_minimum_required(VERSION 3.0)
project(kgraphviewer VERSION 2.3.90)

set(QT_MIN_VERSION "5.4.0")
set(KF5_MIN_VERSION "5.16.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(CMakePackageConfigHelpers)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMInstallIcons)
include(ECMSetupVersion)

# search basic libraries first
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Svg PrintSupport)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DocTools
    Parts
    KIO
    WidgetsAddons
    IconThemes
)

find_package(Boost 1.36 REQUIRED)
find_package(Graphviz REQUIRED)

if(NOT DOT)
   MESSAGE(STATUS "dot executable from Graphviz not found. You will need it at runtime")
endif(NOT DOT)

option(BUILD_GRAPHEDITOR "Build the graph editor app (WIP, not yet functional). [default=OFF]" OFF)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

add_subdirectory(src)
add_subdirectory(doc)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
