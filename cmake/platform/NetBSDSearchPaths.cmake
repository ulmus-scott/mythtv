#
# Copyright (C) 2025 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

if(EXISTS /usr/X11R7)
  list(APPEND CMAKE_MODULE_PATH /usr/X11R7/lib/pkgconfig)
  list(APPEND CMAKE_BUILD_RPATH /usr/X11R7/lib)
  set(QTGUI_CONFIG_EXTRA /usr/pkg/${QT_PKG_NAME_LC}/include)
else()
  message(FATAL_ERROR "X11 must be installed.")
endif()

# Set the RPATH variables so that the libraries under
# /usr/pkg/qt?/lib # can be found. (/usr/pkg/lib is
# automatically added.)  You will still need to set
# "LD_LIBRARY_PATH=/usr/pkg/lib" in your environment
# for Qt to properly load the mysql plugin.
list(APPEND CMAKE_INSTALL_RPATH /usr/pkg/${QT_PKG_NAME_LC}/lib)
list(APPEND CMAKE_BUILD_RPATH /usr/pkg/${QT_PKG_NAME_LC}/lib)

# Ensure that the /usr/pkg/include is always searched
include_directories(SYSTEM AFTER /usr/pkg/include)
