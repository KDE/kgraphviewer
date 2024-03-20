cmake_minimum_required(VERSION 3.16)

project(kgraphviewer VERSION "2.4.3")
set(KGRAPHVIEWERLIB_SOVERION 3)

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.100.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
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
include(ECMDeprecationSettings)
include(ECMSetupVersion)

if (QT_MAJOR_VERSION STREQUAL "6")
    set(QT_MIN_VERSION "6.5.0")
    set(KF_MIN_VERSION "6.0.0")
    set(KF_MAJOR_VERSION "6")
else()
    set(KF_MAJOR_VERSION "5")
endif()

# search basic libraries first
find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
    Widgets
    Svg
    PrintSupport
)
if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat ${QT_MIN_VERSION} REQUIRED)
endif()

find_package(KF${KF_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DocTools
    I18n
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

# added by KDE_COMPILERSETTINGS_LEVEL >= 5.85.0
remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

if (QT_MAJOR_VERSION STREQUAL "6")
    ecm_set_disabled_deprecation_versions(
        QT 6.5
        KF 6.0
    )
else()
    ecm_set_disabled_deprecation_versions(
        QT 5.15.2
        KF 5.100
    )
endif()

add_subdirectory(src)
add_subdirectory(doc)

ki18n_install(po)
if(KF${KF_MAJOR_VERSION}DocTools_FOUND)
    kdoctools_install(po)
endif()

ecm_qt_install_logging_categories(
    EXPORT KGRAPHVIEWER
    FILE kgraphviewer.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
