#
# Copyright (c) 2016-2022 Barbara Geller
# Copyright (c) 2016-2022 Ansel Sermersheim
#
# This file is part of CsLibGuarded.
#
# CsLibGuarded is free software, released under the BSD 2-Clause license.
# For license details refer to LICENSE provided with this project.
#
# CsLibGuarded is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# https://opensource.org/licenses/BSD-2-Clause
#

if(CsLibGuarded_FOUND)
   return()
endif()

set(CsLibGuarded_FOUND TRUE)

# figure out install path
get_filename_component(CsLibGuarded_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(CsLibGuarded_PREFIX ${CsLibGuarded_CMAKE_DIR}/ ABSOLUTE)

# library dependencies (contains definitions for imported targets)
include("${CsLibGuarded_CMAKE_DIR}/CsLibGuardedLibraryTargets.cmake")

# imported targets  INCLUDE_DIRECTORIES
get_target_property(CsLibGuarded_INCLUDES  CsLibGuarded INTERFACE_INCLUDE_DIRECTORIES)
