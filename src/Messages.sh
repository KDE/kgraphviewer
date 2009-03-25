#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui -o -name \*.rc -o -name \*.kcfg` >> rc.cpp
$XGETTEXT *.cpp part/*.cpp -o $podir/kgraphviewer.pot
rm -f rc.cpp
