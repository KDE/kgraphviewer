cmake_minimum_required(VERSION 3.16)

project(kgraphviewer VERSION "2.4.3")
set(KGRAPHVIEWERLIB_SOVERION 3)

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.80.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(CMakePackageConfigHelpers)
include(KDEClangFormat)
include(FeatureSummary)
include(GenerateExportHeader)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

# search basic libraries first
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Svg PrintSupport)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DocTools
    Parts
    WidgetsAddons
)

find_package(Boost 1.36 REQUIRED)
find_package(Graphviz 2.30.0 REQUIRED)

find_package(DOT)
set_package_properties(DOT PROPERTIES
    DESCRIPTION "Tool to draw directed graphs"
    URL "http://www.graphviz.org/"
    PURPOSE "Needed for parsing .dot files"
    TYPE RUNTIME
)

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

ecm_qt_install_logging_categories(
    EXPORT KGRAPHVIEWER
    FILE kgraphviewer.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
