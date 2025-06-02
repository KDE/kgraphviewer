cmake_minimum_required(VERSION 3.16)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "04")
set (RELEASE_SERVICE_VERSION_MICRO "2")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KGRAPHVIEWER_BASE_VERSION once new features are added
set(KGRAPHVIEWER_BASE_VERSION "2.5")
set(KGRAPHVIEWER_VERSION "${KGRAPHVIEWER_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(kgraphviewer VERSION ${KGRAPHVIEWER_VERSION})
set(KGRAPHVIEWERLIB_SOVERSION 0)

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

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

# search basic libraries first
find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus
    Widgets
    Svg
    PrintSupport
    Core5Compat
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Parts
    WidgetsAddons
)

find_package(KF6DocTools ${KF_MIN_VERSION})

find_package(Boost 1.36 REQUIRED)
find_package(Graphviz 2.30.0 REQUIRED)

find_package(DOT)
set_package_properties(DOT PROPERTIES
    DESCRIPTION "Tool to draw directed graphs"
    URL "https://www.graphviz.org/"
    PURPOSE "Needed for parsing .dot files"
    TYPE RUNTIME
)

option(BUILD_GRAPHEDITOR "Build the graph editor app (WIP, not yet functional). [default=OFF]" OFF)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 6.0
)

add_subdirectory(src)

ki18n_install(po)

if(KF6DocTools_FOUND)
    add_subdirectory(doc)
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
