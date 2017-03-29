cmake_minimum_required(VERSION 3.0)
project(kgraphviewer VERSION 2.3.90)

set(ECM_DEP_VERSION "1.6.0")
find_package(ECM ${ECM_DEP_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMPackageConfigHelpers)

# search packages used by KDE
find_package(Qt5 CONFIG REQUIRED Core DBus Widgets Svg PrintSupport)

find_package(Boost 1.36 REQUIRED)
find_package(GraphViz REQUIRED)

set(KF5_DEP_VERSION "5.16.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
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

add_definitions(
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_STRICT_ITERATORS
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

add_subdirectory(src)
add_subdirectory(doc)
