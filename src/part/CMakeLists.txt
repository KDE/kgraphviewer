include_directories( ${CMAKE_SOURCE_DIR}/kgraphviewer/src ${CMAKE_SOURCE_DIR}/kgraphviewer/src/part ${CMAKE_SOURCE_DIR}/kgraphviewer/src/printing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set( kgraphviewerpart_PART_SRCS kgraphviewer_part.cpp graphelement.cpp graphsubgraph.cpp graphnode.cpp graphedge.cpp graphexporter.cpp pannerview.cpp canvassubgraph.cpp canvasnode.cpp canvasedge.cpp dotgraph.cpp dotgraphview.cpp dot2qtconsts.cpp dotgrammar.cpp DotGraphParsingHelper.cpp FontsCache.cpp simpleprintingsettings.cpp simpleprintingengine.cpp simpleprintingcommand.cpp simpleprintingpagesetup.cpp simpleprintpreviewwindow_p.cpp simpleprintpreviewwindow.cpp KgvGlobal.cpp KgvUnit.cpp KgvUnitWidgets.cpp KgvPageLayoutColumns.cpp KgvPageLayoutDia.cpp KgvPageLayout.cpp KgvPageLayoutHeader.cpp KgvPageLayoutSize.cpp )

kde4_add_kcfg_files( kgraphviewerpart_PART_SRCS kgraphviewer_partsettings.kcfgc )

set( kgraphviewerprinting_UI simpleprintingpagesetupbase.ui KgvPageLayoutColumnsBase.ui KgvPageLayoutHeaderBase.ui )

kde4_add_ui_files(kgraphviewerpart_PART_SRCS ${kgraphviewerprinting_UI} )


kde4_add_plugin(kgraphviewerpart ${kgraphviewerpart_PART_SRCS})

target_link_libraries(kgraphviewerpart  ${QT_AND_KDECORE_LIBS} ${QT_QT_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDE4_KHTML_LIBS} ${KDE4_KWALLETCLIENT_LIBS} ${KDE4_KDESU_LIBS} ${KDE4_KDEFX_LIBS}  )

install( TARGETS kgraphviewerpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewer_part.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart )

install( FILES kgraphviewer_part.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kgraphviewer_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kgraphviewer-bev.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newnode.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newedge.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )

