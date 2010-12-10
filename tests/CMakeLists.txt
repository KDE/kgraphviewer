set(KGRAPHVIZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/kgraphviz)
set(KGRAPHVIEWER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KGRAPHVIEWER_SOURCE_DIR}
)

add_subdirectory(kgraphviz)