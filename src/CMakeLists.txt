ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX KGRAPHVIEWER
    SOVERSION ${KGRAPHVIEWERLIB_SOVERION}
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/config-kgraphviewer.h"
)

add_subdirectory( part )

########### kgraphviewer target ###############

add_executable(kgraphviewer)

target_sources(kgraphviewer PRIVATE
    main.cpp
    kgraphviewer.cpp
    kgraphviewerConfigDialog.cpp

    kgraphviewer.qrc
)

set(kgraphviewer_dbus_SRCS)
qt_add_dbus_adaptor(kgraphviewer_dbus_SRCS org.kde.kgraphviewer.xml kgraphviewer.h KGraphViewerWindow)
target_sources(kgraphviewer PRIVATE ${kgraphviewer_dbus_SRCS})

kconfig_add_kcfg_files(kgraphviewer kgraphviewersettings.kcfgc)

ki18n_wrap_ui(kgraphviewer
    preferencesReload.ui
    preferencesParsing.ui
    preferencesOpenInExistingWindow.ui
    preferencesReopenPreviouslyOpenedFiles.ui
    preferencesAppearance.ui
)

ecm_qt_declare_logging_category(kgraphviewer
    HEADER kgraphviewer_debug.h
    IDENTIFIER KGRAPHVIEWER_LOG
    CATEGORY_NAME org.kde.kgraphviewer
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KGraphViewer"
    EXPORT KGRAPHVIEWER
)

target_link_libraries(kgraphviewer PRIVATE
    KF6::CoreAddons
    KF6::Parts
    KF6::I18n
)

install( TARGETS kgraphviewer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install kgraphviewer files ###############

ecm_install_icons(
    ICONS
    ${CMAKE_CURRENT_SOURCE_DIR}/16-apps-kgraphviewer.png
    ${CMAKE_CURRENT_SOURCE_DIR}/32-apps-kgraphviewer.png
    DESTINATION
    ${KDE_INSTALL_ICONDIR}
    THEME hicolor
)

install( FILES kgraphviewersettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR} )
install( PROGRAMS org.kde.kgraphviewer.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
install(FILES org.kde.kgraphviewer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})


if(NOT BUILD_GRAPHEDITOR)
    return()
endif()

add_executable(kgrapheditor)

target_sources(kgrapheditor PRIVATE
    kgrapheditormain.cpp
    kgrapheditor.cpp
    kgrapheditorConfigDialog.cpp
    KGraphEditorNodesTreeWidget.cpp
    KGraphEditorElementTreeWidget.cpp

    kgrapheditor.qrc
)

set(kgrapheditor_dbus_SRCS)
qt_add_dbus_adaptor(kgrapheditor_dbus_SRCS org.kde.kgrapheditor.xml kgrapheditor.h KGraphEditor)
target_sources(kgrapheditor PRIVATE ${kgrapheditor_dbus_SRCS})

kconfig_add_kcfg_files(kgrapheditor kgrapheditorsettings.kcfgc)

ki18n_wrap_ui(kgrapheditor
    preferencesReload.ui
    preferencesParsing.ui
    preferencesOpenInExistingWindow.ui
    preferencesReopenPreviouslyOpenedFiles.ui
)

ecm_qt_declare_logging_category(kgrapheditor
    HEADER kgrapheditor_debug.h
    IDENTIFIER KGRAPHEDITOR_LOG
    CATEGORY_NAME org.kde.kgrapheditor
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KGraphEditor"
    EXPORT KGRAPHVIEWER
)

target_link_libraries(kgrapheditor PRIVATE
    KF6::CoreAddons
    KF6::Parts
    KF6::I18n
)

install( TARGETS kgrapheditor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install kgrapheditor files ###############

install(FILES kgrapheditorsettings.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(PROGRAMS org.kde.kgrapheditor.desktop DESTINATION ${KDE_INSTALL_APPDIR} )


#add_subdirectory(plasmaapplet)

