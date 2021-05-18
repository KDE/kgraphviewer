add_definitions(-DTRANSLATION_DOMAIN=\"kgraphviewer\")

ecm_qt_declare_logging_category(kgraphviewerlib_LIB_DEBUG_SRCS
    HEADER kgraphviewerlib_debug.h
    IDENTIFIER KGRAPHVIEWERLIB_LOG
    CATEGORY_NAME org.kde.kgraphviewer.lib
    DEFAULT_SEVERITY Warning
)

########### next target ###############

add_library(kgraphviewerlib SHARED)

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
        KF5::Parts
        Graphviz::Graphviz
    PRIVATE
        Qt5::Svg
        Qt5::PrintSupport
        KF5::WidgetsAddons
        KF5::IconThemes
        KF5::XmlGui
        KF5::I18n
)

set_target_properties(kgraphviewerlib PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${KGRAPHVIEWER_SOVERSION}
    OUTPUT_NAME kgraphviewer
)

install( TARGETS kgraphviewerlib ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

configure_file(kgraphviewer_part.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop @ONLY)

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

kcoreaddons_desktop_to_json(kgraphviewerpart ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop)

generate_export_header(kgraphviewerpart BASE_NAME kgraphviewer)

target_include_directories(kgraphviewerpart PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_link_libraries(kgraphviewerpart
    kgraphviewerlib
    KF5::Parts
)

install( TARGETS kgraphviewerpart DESTINATION ${KDE_INSTALL_PLUGINDIR})

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KGraphViewerPart")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KGraphViewerPartConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

########### configure AppStream metadata ###############
configure_file(org.kde.libkgraphviewer.metainfo.xml.in "${CMAKE_CURRENT_BINARY_DIR}/org.kde.libkgraphviewer.metainfo.xml" IMMEDIATE @ONLY)

########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} )

install( FILES ${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer_part.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES chain.png DESTINATION ${KDE_INSTALL_DATADIR}/kgraphviewerpart/pics )
install( FILES chain-broken.png DESTINATION ${KDE_INSTALL_DATADIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-bev.png DESTINATION ${KDE_INSTALL_DATADIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newnode.png DESTINATION ${KDE_INSTALL_DATADIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newedge.png DESTINATION ${KDE_INSTALL_DATADIR}/kgraphviewerpart/pics )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/org.kde.libkgraphviewer.metainfo.xml" DESTINATION ${KDE_INSTALL_METAINFODIR} )

install(FILES kgraphviewer_interface.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kgraphviewer/ COMPONENT Devel)

install( FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KGraphViewerPartConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)
