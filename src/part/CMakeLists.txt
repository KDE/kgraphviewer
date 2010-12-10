include_directories( ../ ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIRS} ${graphviz_INCLUDE_DIRECTORIES} )

link_directories(
    ${CMAKE_BINARY_DIR}/lib
    ${LIB_INSTALL_DIR}
)

########### next target ###############

set( kgraphviewerlib_LIB_SRCS simpleprintingsettings.cpp simpleprintingengine.cpp simpleprintingcommand.cpp simpleprintingpagesetup.cpp simpleprintpreviewwindow_p.cpp simpleprintpreviewwindow.cpp KgvGlobal.cpp KgvUnit.cpp KgvUnitWidgets.cpp KgvPageLayoutColumns.cpp KgvPageLayoutDia.cpp KgvPageLayout.cpp KgvPageLayoutHeader.cpp KgvPageLayoutSize.cpp)

kde4_add_kcfg_files( kgraphviewerlib_LIB_SRCS kgraphviewer_partsettings.kcfgc )

set( kgraphviewerprinting_UI simpleprintingpagesetupbase.ui KgvPageLayoutColumnsBase.ui KgvPageLayoutHeaderBase.ui )

kde4_add_ui_files(kgraphviewerlib_LIB_SRCS ${kgraphviewerprinting_UI} )

kde4_add_library(kgraphviewerlib SHARED ${kgraphviewerlib_LIB_SRCS})

set(kgraphviewerlib_DEPENDS "kgraphviz")

add_definitions(-DQT_STL)
target_link_libraries(kgraphviewerlib ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KWALLETCLIENT_LIBS} ${KDE4_KDESU_LIBS} ${graphviz_LIBRARIES} kgraphviz)

set_target_properties(kgraphviewerlib PROPERTIES VERSION 2.1 SOVERSION 3 OUTPUT_NAME kgraphviewer )

install( TARGETS kgraphviewerlib ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set( kgraphviewerpart_PART_SRCS kgraphviewer_part.cpp  )

kde4_add_plugin(kgraphviewerpart ${kgraphviewerpart_PART_SRCS})

add_definitions(-DQT_STL)
target_link_libraries(kgraphviewerpart ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KWALLETCLIENT_LIBS} ${KDE4_KDESU_LIBS} gvc graph pathplan cdt kgraphviewerlib kgraphviz)

install( TARGETS kgraphviewerpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewer_part.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart )

install( FILES kgraphviewer_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kgraphviewer-bev.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newnode.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
install( FILES kgraphviewer-newedge.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart/pics )
