SUBDIRS = printing part

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kgraphviewer/src -I$(top_srcdir)/kgraphviewer/src/printing $(all_includes)


####################################
# Application Section
####################################
bin_PROGRAMS = kgraphviewer

kgraphviewer_SOURCES = preferencesReload.ui \
preferencesOpenInExistingWindow.ui \
preferencesReopenPreviouslyOpenedFiles.ui \
main.cpp \
kgraphviewer.cpp \
kgraphviewerConfigDialog.cpp \
kgraphviewersettings.kcfgc \
dcopkgraphvieweriface.skel 


kgraphviewer_METASOURCES = AUTO

kgraphviewer_LDFLAGS = $(all_libraries) $(KDE_RPATH)

kgraphviewer_LDADD = $(LIB_QT) $(LIB_KIO) $(LIB_KDECORE) $(LIB_KDEPRINT) $(LIB_KPARTS) $(LIB_KHTML)


##kde_icon_DATA = hi32-app-kgraphviewer.png hi16-app-kgraphviewer.png 
icondir = $(kde_icondir)
icon_ICON = kgraphviewer

kde_apps_DATA = kgraphviewer.desktop

kde_kcfg_DATA = kgraphviewersettings.kcfg

kde_mime_DATA = x-kgraphviewer.desktop

rcdir = $(kde_datadir)/kgraphviewer
rc_DATA = kgraphviewer_shell.rc

