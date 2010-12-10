include_directories(
  ${KGRAPHVIZ_SOURCE_DIR}
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

set(dotgraphtests_SRCS
  dotgraphtests.cpp
)

kde4_add_unit_test(dotgraphtests ${dotgraphtests_SRCS} )
target_link_libraries(dotgraphtests ${KDE4_KDECORE_LIBS} ${QT_QTTEST_LIBRARY} kgraphviz)
