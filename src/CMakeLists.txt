add_subdirectory( part )

include_directories( part ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${graphviz_INCLUDE_DIRECTORIES} )


########### kgraphviewer target ###############

set(kgraphviewer_SRCS main.cpp kgraphviewer.cpp kgraphviewerConfigDialog.cpp)

qt4_add_dbus_adaptor(kgraphviewer_SRCS org.kde.kgraphviewer.xml kgraphviewer.h KGraphViewerWindow)

kde4_add_kcfg_files( kgraphviewer_SRCS kgraphviewersettings.kcfgc )

kde4_add_ui_files(
  kgraphviewer_SRCS
  preferencesReload.ui
  preferencesParsing.ui
  preferencesOpenInExistingWindow.ui
  preferencesReopenPreviouslyOpenedFiles.ui
  preferencesAppearance.ui
)

kde4_add_executable(kgraphviewer ${kgraphviewer_SRCS})

target_link_libraries(kgraphviewer ${KDE4_KPARTS_LIBS})

install( TARGETS kgraphviewer ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install kgraphviewer files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

install( FILES kgraphviewersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewerui.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewer )
install( PROGRAMS kgraphviewer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


########### kgrapheditor target ###############

set(kgrapheditor_SRCS kgrapheditormain.cpp kgrapheditor.cpp kgrapheditorConfigDialog.cpp KGraphEditorNodesTreeWidget.cpp KGraphEditorElementTreeWidget.cpp)

qt4_add_dbus_adaptor(kgrapheditor_SRCS org.kde.kgrapheditor.xml kgrapheditor.h KGraphEditor)

kde4_add_kcfg_files( kgrapheditor_SRCS kgrapheditorsettings.kcfgc )

kde4_add_ui_files( kgrapheditor_SRCS preferencesReload.ui preferencesParsing.ui preferencesOpenInExistingWindow.ui preferencesReopenPreviouslyOpenedFiles.ui )

kde4_add_executable(kgrapheditor ${kgrapheditor_SRCS})

target_link_libraries(kgrapheditor ${KDE4_KPARTS_LIBS})

install( TARGETS kgrapheditor ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install kgrapheditor files ###############

install( FILES kgrapheditorsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgrapheditorui.rc DESTINATION ${DATA_INSTALL_DIR}/kgrapheditor )
install( PROGRAMS kgrapheditor.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kgraphviewer.appdata.xml DESTINATION ${SHARE_INSTALL_PREFIX}/appdata/ )


#add_subdirectory(plasmaapplet)

