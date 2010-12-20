# Improves speed of string concatenation
add_definitions(-DQT_USE_FAST_CONCATENATION)
add_definitions(-DQT_USE_FAST_OPERATOR_PLUS)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}

  ${Boost_INCLUDE_DIRS}
  ${graphviz_INCLUDE_DIRECTORIES}
)

link_directories(
    ${CMAKE_BINARY_DIR}/lib
    ${LIB_INSTALL_DIR}
)

set(kgraphvz_LIB_SRCS
  canvasedge.cpp
  canvaselement.cpp
  canvasnode.cpp
  canvassubgraph.cpp

  graphedge.cpp
  graphexporter.cpp
  graphelement.cpp
  graphio.cpp
  graphnode.cpp
  graphsubgraph.cpp
  dotgraph.cpp

  dotgraphview.cpp
  pannerview.cpp

  support/loadagraphthread.cpp
  support/layoutagraphthread.cpp
  support/dot2qtconsts.cpp
  support/dotgrammar.cpp
  support/dotgraphparsinghelper.cpp
  support/fontscache.cpp
)

kde4_add_library(kgraphviz SHARED ${kgraphvz_LIB_SRCS})

add_definitions(-DQT_STL)
target_link_libraries(kgraphviz ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KFILE_LIBRARY} ${KDE4_KDECORE_LIBRARY} ${graphviz_LIBRARIES})
set_target_properties(kgraphviz PROPERTIES VERSION 1.0 SOVERSION 1 OUTPUT_NAME kgraphviz )


### install
install(TARGETS kgraphviz ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  kgraphviz_export.h

  canvasedge.h
  canvaselement.h
  canvasnode.h
  canvassubgraph.h

  graphedge.h
  graphexporter.h
  graphelement.h
  graphio.h
  graphnode.h
  graphsubgraph.h
  dotgraph.h

  dotgraphview.h
  pannerview.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kgraphviz COMPONENT Devel
)

install(FILES
  # TODO: Get rid off this header here
  support/dotrenderop.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kgraphviz/support COMPONENT Devel
)
