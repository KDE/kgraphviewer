#add_subdirectory( printing ) 
add_subdirectory( part ) 

include_directories( ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/part ${CMAKE_SOURCE_DIR}/src/printing ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kgraphviewer_SRCS main.cpp kgraphviewer.cpp kgraphviewerConfigDialog.cpp)

qt4_add_dbus_adaptor(kgraphviewer_SRCS org.kde.kgraphviewer.xml kgraphviewer.h KGraphViewer)

kde4_add_kcfg_files( kgraphviewer_SRCS kgraphviewersettings.kcfgc )

kde4_add_ui_files( kgraphviewer_SRCS preferencesReload.ui preferencesOpenInExistingWindow.ui preferencesReopenPreviouslyOpenedFiles.ui )

kde4_add_executable(kgraphviewer ${kgraphviewer_SRCS})

target_link_libraries(kgraphviewer  ${QT_AND_KDECORE_LIBS} ${KDE4_KHTML_LIBS})

install( TARGETS kgraphviewer DESTINATION ${BIN_INSTALL_DIR})

########### install files ###############

kde4_install_icons( ${ICON_INSTALL_DIR} )

#TODO move x-kgraphviewer mimetype to mime (ask to David Faure)
install( FILES x-kgraphviewer.desktop DESTINATION ${MIME_INSTALL_DIR}/application )
install( FILES kgraphviewersettings.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
install( FILES kgraphviewerui.rc DESTINATION ${DATA_INSTALL_DIR}/kgraphviewer )
install( FILES kgraphviewer.desktop DESTINATION ${XDG_APPS_DIR} )
install( FILES kgraphviewer.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

#SUBDIRS = printing part
#
#INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kgraphviewer/src -I$(top_srcdir)/kgraphviewer/src/printing $(all_includes)
#
#
#####################################
## Application Section
#####################################
#bin_PROGRAMS = kgraphviewer
#
#kgraphviewer_SOURCES = preferencesReload.ui \
#preferencesOpenInExistingWindow.ui \
#preferencesReopenPreviouslyOpenedFiles.ui \
#main.cpp \
#kgraphviewer.cpp \
#kgraphviewerConfigDialog.cpp \
#kgraphviewersettings.kcfgc \
#dcopkgraphvieweriface.skel 
#
#
#kgraphviewer_METASOURCES = AUTO
#
#kgraphviewer_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#kgraphviewer_LDADD = $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE) $(LIB_KDEPRINT) $(LIB_KPARTS) $(LIB_KHTML)
#
#
###kde_icon_DATA = hi32-app-kgraphviewer.png hi16-app-kgraphviewer.png 
#icondir = $(kde_icondir)
#icon_ICON = kgraphviewer
#
#kde_apps_DATA = kgraphviewer.desktop
#
#kde_kcfg_DATA = kgraphviewersettings.kcfg
#
#kde_mime_DATA = x-kgraphviewer.desktop
#
#rcdir = $(kde_datadir)/kgraphviewer
