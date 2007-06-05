METASOURCES = AUTO

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kgraphviewer/src  -I$(top_srcdir)/kgraphviewer/src/part -I$(top_srcdir)/kgraphviewer/src/printing $(all_includes)


##################################
# Part Section
##################################
kde_module_LTLIBRARIES = libkgraphviewerpart.la

libkgraphviewerpart_la_SOURCES = kgraphviewer_part.cpp \
graphsubgraph.cpp \
graphnode.cpp \
graphedge.cpp \
graphexporter.cpp \
pannerview.cpp \
canvassubgraph.cpp \
canvasnode.cpp \
canvasedge.cpp \
dotgraph.cpp \
dotgraphview.cpp \
dot2qtconsts.cpp \
dotgrammar.cpp \
FontsCache.cpp \
kgraphviewer_partsettings.kcfgc

libkgraphviewerpart_la_LIBADD = ../printing/libkgraphviewerprinting.la 
## $(top_builddir)/kdeprint/libkdeprint.la $(top_builddir)/kutils/libkutils.la $(top_builddir)/kjs/libkjs.la $(LUA_LIBS)

libkgraphviewerpart_la_LDFLAGS = $(all_libraries) $(LIB_QT) $(LIB_KPARTS) $(LIB_KHTML)


# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use kde_icondir.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.

kde_apps_DATA = kgraphviewer_part.desktop

kde_kcfg_DATA = kgraphviewer_partsettings.kcfg

rcdir = $(kde_datadir)/kgraphviewerpart
rc_DATA = kgraphviewer_part.rc

kgraphviewerpartpicsdir = $(kde_datadir)/kgraphviewer/pics
kgraphviewerpartpics_DATA = kgraphviewer-bev.png

