add_definitions(-DTRANSLATION_DOMAIN=\"kgraphviewer\")

ecm_qt_declare_logging_category(kgraphviewerlib_LIB_DEBUG_SRCS
    HEADER kgraphviewerlib_debug.h
    IDENTIFIER KGRAPHVIEWERLIB_LOG
    CATEGORY_NAME org.kde.kgraphviewer.lib
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KGraphViewer (Library and KPart)"
    EXPORT KGRAPHVIEWER
)

########### next target ###############

add_library(kgraphviewerlib SHARED)

set_target_properties(kgraphviewerlib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${KGRAPHVIEWER_SOVERSION}
    OUTPUT_NAME kgraphviewer
)

target_sources(kgraphviewerlib PRIVATE
    loadagraphthread.cpp
    layoutagraphthread.cpp
    graphelement.cpp
    graphsubgraph.cpp
    graphnode.cpp
    graphedge.cpp
    graphexporter.cpp
    pannerview.cpp
    canvassubgraph.cpp
    canvasnode.cpp
    canvasedge.cpp
    canvaselement.cpp
    dotgraph.cpp
    dotgraphview.cpp
    dot2qtconsts.cpp
    dotgrammar.cpp
    DotGraphParsingHelper.cpp
    FontsCache.cpp
    simpleprintingsettings.cpp
    simpleprintingengine.cpp
    simpleprintingcommand.cpp
    simpleprintingpagesetup.cpp
    simpleprintpreviewwindow_p.cpp
    simpleprintpreviewwindow.cpp
    KgvGlobal.cpp
    KgvUnit.cpp
    KgvUnitWidgets.cpp
    KgvPageLayoutColumns.cpp
    KgvPageLayoutDia.cpp
    KgvPageLayout.cpp
    KgvPageLayoutHeader.cpp
    KgvPageLayoutSize.cpp
    ${kgraphviewerlib_LIB_DEBUG_SRCS}
)

kconfig_add_kcfg_files(kgraphviewerlib kgraphviewer_partsettings.kcfgc)

ki18n_wrap_ui(kgraphviewerlib
    simpleprintingpagesetupbase.ui
    KgvPageLayoutColumnsBase.ui
    KgvPageLayoutHeaderBase.ui
)

target_include_directories(kgraphviewerlib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(kgraphviewerlib
    PUBLIC
        KF${KF_MAJOR_VERSION}::Parts
        Graphviz::Graphviz
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Svg
        Qt${QT_MAJOR_VERSION}::PrintSupport
        KF${KF_MAJOR_VERSION}::WidgetsAddons
        KF${KF_MAJOR_VERSION}::XmlGui
        KF${KF_MAJOR_VERSION}::I18n
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(kgraphviewerlib
        PRIVATE
            Qt6::Core5Compat
    )
endif()

install( TARGETS kgraphviewerlib ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

if (QT_MAJOR_VERSION STREQUAL "6")
    configure_file(kgraphviewer_part.json.in ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.json @ONLY)
else()
    configure_file(kgraphviewer_part.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop @ONLY)
endif()

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KGRAPHVIEWERPART
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfigVersion.cmake"
)

add_library(kgraphviewerpart MODULE)

target_sources(kgraphviewerpart PRIVATE
    kgraphviewer_part.cpp
    ${kgraphviewerlib_LIB_DEBUG_SRCS}

    kgraphviewer_part.qrc
)

if (QT_MAJOR_VERSION STREQUAL "5")
    kcoreaddons_desktop_to_json(kgraphviewerpart ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop)
endif()

generate_export_header(kgraphviewerpart BASE_NAME kgraphviewer)

target_include_directories(kgraphviewerpart PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries(kgraphviewerpart
    kgraphviewerlib
    KF${KF_MAJOR_VERSION}::Parts
    KF${KF_MAJOR_VERSION}::I18n
)

add_library(KGraphViewerPart INTERFACE)

target_include_directories(KGraphViewerPart INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/kgraphviewer>")

install(TARGETS KGraphViewerPart EXPORT KGraphViewerPartTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KGraphViewerPart")
install(EXPORT KGraphViewerPartTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KGraphViewerPartTargets.cmake)

if (QT_MAJOR_VERSION STREQUAL "6")
    install(TARGETS kgraphviewerpart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)
else()
    install(TARGETS kgraphviewerpart DESTINATION ${KDE_INSTALL_PLUGINDIR})
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KGraphViewerPartConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} )

if (QT_MAJOR_VERSION STREQUAL "5")
    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR} )
endif()

install(FILES kgraphviewer_interface.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kgraphviewer/ COMPONENT Devel)

install( FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)
