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

set(CsLibGuarded_VERSION_MAJOR "@BUILD_MAJOR@" PARENT_SCOPE)
set(CsLibGuarded_VERSION_MINOR "@BUILD_MINOR@" PARENT_SCOPE)
set(CsLibGuarded_VERSION_PATCH "@BUILD_MICRO@" PARENT_SCOPE)

set(CsLibGuarded_VERSION       "@BUILD_MAJOR@.@BUILD_MINOR@.@BUILD_MICRO@" PARENT_SCOPE)
set(CsLibGuarded_VERSION_API   "@BUILD_MAJOR@.@BUILD_MINOR@" PARENT_SCOPE)

set(PACKAGE_VERSION "@BUILD_MAJOR@.@BUILD_MINOR@.@BUILD_MICRO@")
