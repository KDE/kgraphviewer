SUBDIRS = src 


messages: rc.cpp
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
	$(XGETTEXT) `find . -name "*.cpp" -o -name "*.h"` -o $(podir)/kgraphviewer.pot

