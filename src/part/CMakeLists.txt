include_directories( ../ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set( kgraphviewerpart_PART_SRCS kgraphviewer_part.cpp graphelement.cpp graphsubgraph.cpp graphnode.cpp graphedge.cpp graphexporter.cpp pannerview.cpp canvassubgraph.cpp canvasnode.cpp canvasedge.cpp canvaselement.cpp dotgraph.cpp dotgraphview.cpp dot2qtconsts.cpp dotgrammar.cpp DotGraphParsingHelper.cpp FontsCache.cpp simpleprintingsettings.cpp simpleprintingengine.cpp simpleprintingcommand.cpp simpleprintingpagesetup.cpp simpleprintpreviewwindow_p.cpp simpleprintpreviewwindow.cpp KgvGlobal.cpp KgvUnit.cpp KgvUnitWidgets.cpp KgvPageLayoutColumns.cpp KgvPageLayoutDia.cpp KgvPageLayout.cpp KgvPageLayoutHeader.cpp KgvPageLayoutSize.cpp )

kde4_add_kcfg_files( kgraphviewerpart_PART_SRCS kgraphviewer_partsettings.kcfgc )

set( kgraphviewerprinting_UI simpleprintingpagesetupbase.ui KgvPageLayoutColumnsBase.ui KgvPageLayoutHeaderBase.ui )

kde4_add_ui_files(kgraphviewerpart_PART_SRCS ${kgraphviewerprinting_UI} )


kde4_add_plugin(kgraphviewerpart ${kgraphviewerpart_PART_SRCS})

add_definitions(-DQT_STL)
target_link_libraries(kgraphviewerpart  ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KWALLETCLIENT_LIBS} ${KDE4_KDESU_LIBS} gvc graph pathplan cdt)

install( TARGETS kgraphviewerpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewer_part.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart )

install( FILES kgraphviewer_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kgraphviewer-bev.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newnode.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newedge.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )

