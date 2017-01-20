include_directories( ../ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${graphviz_INCLUDE_DIRECTORIES} )

########### next target ###############

set( kgraphviewerlib_LIB_SRCS loadagraphthread.cpp layoutagraphthread.cpp graphelement.cpp graphsubgraph.cpp graphnode.cpp graphedge.cpp graphexporter.cpp pannerview.cpp canvassubgraph.cpp canvasnode.cpp canvasedge.cpp canvaselement.cpp dotgraph.cpp dotgraphview.cpp dot2qtconsts.cpp dotgrammar.cpp DotGraphParsingHelper.cpp FontsCache.cpp simpleprintingsettings.cpp simpleprintingengine.cpp simpleprintingcommand.cpp simpleprintingpagesetup.cpp simpleprintpreviewwindow_p.cpp simpleprintpreviewwindow.cpp KgvGlobal.cpp KgvUnit.cpp KgvUnitWidgets.cpp KgvPageLayoutColumns.cpp KgvPageLayoutDia.cpp KgvPageLayout.cpp KgvPageLayoutHeader.cpp KgvPageLayoutSize.cpp)

kde4_add_kcfg_files( kgraphviewerlib_LIB_SRCS kgraphviewer_partsettings.kcfgc )

set( kgraphviewerprinting_UI simpleprintingpagesetupbase.ui KgvPageLayoutColumnsBase.ui KgvPageLayoutHeaderBase.ui )

kde4_add_ui_files(kgraphviewerlib_LIB_SRCS ${kgraphviewerprinting_UI} )


kde4_add_library(kgraphviewerlib SHARED ${kgraphviewerlib_LIB_SRCS})

add_definitions(-DQT_STL)
target_link_libraries(kgraphviewerlib ${QT_QTSVG_LIBRARY} ${KDE4_KIO_LIBS} ${graphviz_LIBRARIES})

set_target_properties(kgraphviewerlib PROPERTIES VERSION ${KGRAPHVIEWER_LIB_VERSION} SOVERSION ${KGRAPHVIEWER_SOVERSION} OUTPUT_NAME kgraphviewer )

install( TARGETS kgraphviewerlib ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set( kgraphviewerpart_PART_SRCS kgraphviewer_part.cpp  )

kde4_add_plugin(kgraphviewerpart ${kgraphviewerpart_PART_SRCS})

add_definitions(-DQT_STL)
target_link_libraries(kgraphviewerpart ${KDE4_KPARTS_LIBS} gvc cgraph pathplan cdt kgraphviewerlib)

install( TARGETS kgraphviewerpart DESTINATION ${PLUGIN_INSTALL_DIR})

########### configure AppStream metadata ###############
configure_file(kgraphviewer.metainfo.xml.in "${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer.metainfo.xml" IMMEDIATE @ONLY)

########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewer_part.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart )

install( FILES kgraphviewer_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES chain.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES chain-broken.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-bev.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newnode.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newedge.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/kgraphviewer.metainfo.xml" DESTINATION ${SHARE_INSTALL_PREFIX}/metainfo/ )

install(FILES kgraphviewer_interface.h DESTINATION ${INCLUDE_INSTALL_DIR}/kgraphviewer/ COMPONENT Devel)
install(FILES dotgraphview.h DESTINATION ${INCLUDE_INSTALL_DIR}/kgraphviewer/ COMPONENT Devel)
install(FILES kgraphviewer_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/kgraphviewer/ COMPONENT Devel)
