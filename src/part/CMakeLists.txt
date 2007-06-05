include_directories( ${CMAKE_SOURCE_DIR}/kgraphviewer/src ${CMAKE_SOURCE_DIR}/kgraphviewer/src/part ${CMAKE_SOURCE_DIR}/kgraphviewer/src/printing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set( kgraphviewerpart_PART_SRCS kgraphviewer_part.cpp graphsubgraph.cpp graphnode.cpp graphedge.cpp graphexporter.cpp pannerview.cpp canvassubgraph.cpp canvasnode.cpp canvasedge.cpp dotgraph.cpp dotgraphview.cpp dot2qtconsts.cpp dotgrammar.cpp FontsCache.cpp simpleprintingsettings.cpp simpleprintingengine.cpp simpleprintingcommand.cpp simpleprintingpagesetup.cpp simpleprintpreviewwindow_p.cpp simpleprintpreviewwindow.cpp KgvGlobal.cpp KgvUnit.cpp KgvUnitWidgets.cpp KgvPageLayoutColumns.cpp KgvPageLayoutDia.cpp KgvPageLayout.cpp KgvPageLayoutHeader.cpp KgvPageLayoutSize.cpp )

kde4_add_kcfg_files( kgraphviewerpart_PART_SRCS kgraphviewer_partsettings.kcfgc )

set( kgraphviewerprinting_UI simpleprintingpagesetupbase.ui KgvPageLayoutColumnsBase.ui KgvPageLayoutHeaderBase.ui )

kde4_add_ui_files(kgraphviewerpart_PART_SRCS ${kgraphviewerprinting_UI} )


kde4_automoc(${kgraphviewerpart_PART_SRCS})

kde4_add_plugin(kgraphviewerpart ${kgraphviewerpart_PART_SRCS})

target_link_libraries(kgraphviewerpart  ${QT_AND_KDECORE_LIBS} ${QT_QT_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} ${QT_QTSVG_LIBRARY} ${KDE4_KHTML_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KDEPRINT_LIBS} ${KDE4_KWALLETCLIENT_LIBS} ${KDE4_KDESU_LIBS} ${KDE4_KDEFX_LIBS}  )

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kgraphviewerpart )

install( TARGETS kgraphviewerpart DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kgraphviewer_partsettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewer_part.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewerpart )

install( FILES kgraphviewer_part.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES kgraphviewer_part.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kgraphviewer-bev.png DESTINATION ${DATA_INSTALL_DIR}/kgraphviewer/pics )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kgraphviewer/src  -I$(top_srcdir)/kgraphviewer/src/part -I$(top_srcdir)/kgraphviewer/src/printing $(all_includes)
#
#
###################################
## Part Section
###################################
#kde_module_LTLIBRARIES = libkgraphviewerpart.la
#
#libkgraphviewerpart_la_SOURCES = kgraphviewer_part.cpp \
#graphsubgraph.cpp \
#graphnode.cpp \
#graphedge.cpp \
#graphexporter.cpp \
#pannerview.cpp \
#canvassubgraph.cpp \
#canvasnode.cpp \
#canvasedge.cpp \
#dotgraph.cpp \
#dotgraphview.cpp \
#dot2qtconsts.cpp \
#dotgrammar.cpp \
#FontsCache.cpp \
#kgraphviewer_partsettings.kcfgc
#
#libkgraphviewerpart_la_LIBADD = ../printing/libkgraphviewerprinting.la 
### $(top_builddir)/kdeprint/libkdeprint.la $(top_builddir)/kutils/libkutils.la $(top_builddir)/kjs/libkjs.la $(LUA_LIBS)
#
#libkgraphviewerpart_la_LDFLAGS = $(all_libraries) $(LIB_QT) $(LIB_KPARTS) $(LIB_KHTML)
#
#
## These paths are KDE specific. Use them:
## kde_appsdir         Where your application's menu entry (.desktop) should go to.
## kde_icondir         Where your icon should go to - better use kde_icondir.
## kde_sounddir        Where your sounds should go to.
## kde_htmldir         Where your docs should go to. (contains lang subdirs)
## kde_datadir         Where you install application data. (Use a subdir)
## kde_locale          Where translation files should go to. (contains lang subdirs)
## kde_cgidir          Where cgi-bin executables should go to.
## kde_confdir         Where config files should go to (system-wide ones with default values).
## kde_mimedir         Where mimetypes .desktop files should go to.
## kde_servicesdir     Where services .desktop files should go to.
## kde_servicetypesdir Where servicetypes .desktop files should go to.
#
#kde_apps_DATA = kgraphviewer_part.desktop
#
#kde_kcfg_DATA = kgraphviewer_partsettings.kcfg
#
#rcdir = $(kde_datadir)/kgraphviewerpart
#rc_DATA = kgraphviewer_part.rc
#
#kgraphviewerpartpicsdir = $(kde_datadir)/kgraphviewer/pics
#kgraphviewerpartpics_DATA = kgraphviewer-bev.png
#
