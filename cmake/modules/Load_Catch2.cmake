# ***********************************************************************
#
# Copyright (c) 2016-2023 Barbara Geller
# Copyright (c) 2016-2023 Ansel Sermersheim
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
# ***********************************************************************

find_package(Catch2 QUIET)

if (NOT TARGET Catch2::Catch2)
   message(STATUS "Catch2 was not found, CsLibGuarded unit tests will not be built\n")
   return()
endif()
