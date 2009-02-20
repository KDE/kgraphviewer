#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.ui -o -name \*.rc` >> rc.cpp
$XGETTEXT *.cpp part/*.cpp -o $podir/kgraphviewer.pot
