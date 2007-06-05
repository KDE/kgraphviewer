INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/kgraphviewer/src/part -I$(top_srcdir)/kgraphviewer/src/printing $(all_includes)

## let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

lib_LTLIBRARIES = libkgraphviewerprinting.la
libkgraphviewerprinting_la_SOURCES = simpleprintingpagesetupbase.ui \
simpleprintingsettings.cpp \
simpleprintingengine.cpp \
simpleprintingcommand.cpp \
simpleprintingpagesetup.cpp \
simpleprintpreviewwindow_p.cpp \
simpleprintpreviewwindow.cpp \
KgvGlobal.cpp \
KgvGenStyles.cpp \
KgvUnit.cpp \
KgvUnitWidgets.cpp \
KgvPageLayoutColumnsBase.ui \
KgvPageLayoutHeaderBase.ui \
KgvPageLayoutColumns.cpp \
KgvPageLayoutDia.cpp \
KgvPageLayout.cpp \
KgvPageLayoutHeader.cpp \
KgvPageLayoutSize.cpp

libkgraphviewerprinting_la_LDFLAGS = $(all_libraries)
libkgraphviewerprinting_la_LIBADD = $(LIB_KDECORE) $(LIB_QT) $(LIB_KIO) $(LIB_KPARTS) $(LIB_KHML) $(LIB_KDEPRINT)

kgraphviewerprintingincludedir = $(includedir) $(includedir)/src $(includedir)/printing

## kgraphviewerprintinginclude_HEADERS = catalog.h tag.h


kgraphviewerpartpicsdir = $(kde_datadir)/kgraphviewer/pics
kgraphviewerpartpics_DATA = chain.png chain-broken.png

