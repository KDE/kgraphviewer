# - Try to find GraphViz
# Once done this will define
#
#  GRAPHVIZ_FOUND - System has GraphViz
#  GRAPHVIZ_INCLUDE_DIR - The GraphViz include directory
#  GRAPHVIZ_LIBRARIES - The libraries needed to use GraphViz
#  GRAPHVIZ_DEFINITIONS - Compiler switches required for using GraphViz
#  GRAPHVIZ_DOT_EXECUTABLE - The Dot executable

# Copyright (c) 2010, Milian Wolff <mail@milianw.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


IF (GRAPHVIZ_INCLUDE_DIR AND GRAPHVIZ_LIBRARIES)
   # in cache already
   SET(GraphViz_FIND_QUIETLY TRUE)
ENDIF (GRAPHVIZ_INCLUDE_DIR AND GRAPHVIZ_LIBRARIES)

IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_GRAPHVIZ QUIET libgraph)
   SET(GRAPHVIZ_DEFINITIONS ${PC_GRAPHVIZ_CFLAGS_OTHER})
ENDIF (NOT WIN32)

FIND_PATH(GRAPHVIZ_INCLUDE_DIR graphviz/graph.h
   HINTS
   ${PC_GRAPHVIZ_INCLUDEDIR}
   ${PC_GRAPHVIZ_INCLUDE_DIRS}
   PATH_SUFFIXES graphviz
   )

FOREACH(LIB gvc graph pathplan cdt)
  FIND_LIBRARY(temp_${LIB} NAMES ${LIB}
    HINTS
    ${PC_GRAPHVIZ_LIBDIR}
    ${PC_GRAPHVIZ_LIBRARY_DIRS}
  )
  list(APPEND GRAPHVIZ_LIBRARIES ${temp_${LIB}})
ENDFOREACH(LIB gvc graph pathplan cdt)

MESSAGE(STATUS ${GRAPHVIZ_LIBRARIES})

FIND_PROGRAM(GRAPHVIZ_DOT_EXECUTABLE dot  )

INCLUDE(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set GRAPHVIZ_FOUND to TRUE if 
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GraphViz DEFAULT_MSG GRAPHVIZ_LIBRARIES GRAPHVIZ_INCLUDE_DIR)

MARK_AS_ADVANCED(GRAPHVIZ_INCLUDE_DIR GRAPHVIZ_LIBRARIES GRAPHVIZ_XMLLINT_EXECUTABLE)

