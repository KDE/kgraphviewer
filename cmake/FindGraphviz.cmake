# This cmake file comes from MOAB:
# MOAB, a Mesh-Oriented datABase, is a software component for creating,
# storing and accessing finite element mesh data.
# 
# Copyright 2004 Sandia Corporation.  Under the terms of Contract
# DE-AC04-94AL85000 with Sandia Coroporation, the U.S. Government
# retains certain rights in this software.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.  A copy of the full
# GNU Lesser General Public License can be found at
# http://www.gnu.org/copyleft/lesser.html.
# 
# For more information, contact the authors of this software at
# moab@sandia.gov.
#
# Find Graphviz libraries.  This will set the following variables:
#     Graphviz_LIBRARIES
#     Graphviz_FOUND
#     Graphviz_INCLUDE_DIRECTORIES
#     Graphviz_VERSION

find_package(PkgConfig)
pkg_check_modules(pc_graphviz ${REQUIRED} libgvc libcdt libcgraph libpathplan)
    
find_path(Graphviz_INCLUDE_DIRECTORIES
    NAMES gvc.h
    HINTS ${pc_graphviz_INCLUDEDIR}
          ${pc_graphviz_INCLUDE_DIRS}
    )
    
find_library(Graphviz_GVC_LIBRARY
    NAMES gvc
    HINTS ${pc_graphviz_LIBDIR}
          ${pc_graphviz_LIBRARY_DIRS}
    )

find_library(Graphviz_CDT_LIBRARY
    NAMES cdt
    HINTS ${pc_graphviz_LIBDIR}
          ${pc_graphviz_LIBRARY_DIRS}
    )

find_library(Graphviz_GRAPH_LIBRARY
    NAMES cgraph
    HINTS ${pc_graphviz_LIBDIR}
          ${pc_graphviz_LIBRARY_DIRS}
    )

find_library(Graphviz_PATHPLAN_LIBRARY
    NAMES pathplan
    HINTS ${pc_graphviz_LIBDIR}
          ${pc_graphviz_LIBRARY_DIRS}
    )

set(Graphviz_LIBRARIES
    "${Graphviz_GVC_LIBRARY}" "${Graphviz_CDT_LIBRARY}"
    "${Graphviz_GRAPH_LIBRARY}" "${Graphviz_PATHPLAN_LIBRARY}")

if (EXISTS "${Graphviz_INCLUDE_DIRECTORIES}/graphviz_version.h")
    file(READ "${Graphviz_INCLUDE_DIRECTORIES}/graphviz_version.h" _graphviz_version_content)
    string(REGEX MATCH "#define +PACKAGE_VERSION +\"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _dummy "${_graphviz_version_content}")
    set(Graphviz_VERSION "${CMAKE_MATCH_1}")
endif ()

if ("${Graphviz_FIND_VERSION}" VERSION_GREATER "${Graphviz_VERSION}")
    message(FATAL_ERROR "Required version (" ${Graphviz_FIND_VERSION} ") is higher than found version (" ${Graphviz_VERSION} ")")
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Graphviz
    REQUIRED_VARS Graphviz_LIBRARIES Graphviz_INCLUDE_DIRECTORIES
    VERSION_VAR   Graphviz_VERSION)
